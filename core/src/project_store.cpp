#include "openlab/core/project_store.hpp"

#include <json.hpp>
#include <miniz.h>

#include <array>
#include <fstream>
#include <limits>
#include <set>
#include <system_error>
#include <type_traits>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace openlab::core {
namespace {
using Json = nlohmann::json;
constexpr std::size_t maxArchiveBytes = 32U * 1024U * 1024U;
constexpr std::size_t maxEntryBytes = 16U * 1024U * 1024U;
constexpr std::size_t maxComponents = 100000;
constexpr std::size_t maxDefinitions = 10000;
constexpr std::size_t maxConnections = 500000;

Error ioError(std::string message) { return makeError(ErrorCode::Io, std::move(message)); }
Json dimensionJson(const Dimension& d) { return d.exponents(); }
Result<Dimension> readDimension(const Json& value) {
    if (!value.is_array() || value.size() != static_cast<std::size_t>(BaseDimension::Count))
        return Result<Dimension>::failure(makeError(ErrorCode::CorruptData, "Dimension must contain seven SI exponents."));
    Dimension::Exponents e{};
    for (std::size_t i = 0; i < e.size(); ++i) {
        if (!value[i].is_number_integer()) return Result<Dimension>::failure(makeError(ErrorCode::CorruptData, "SI exponent is not an integer."));
        const int n = value[i].get<int>();
        if (n < -32 || n > 32) return Result<Dimension>::failure(makeError(ErrorCode::ResourceLimit, "SI exponent exceeds supported range."));
        e[i] = static_cast<std::int8_t>(n);
    }
    return Result<Dimension>::success(Dimension(e));
}
Json definitionJson(const ComponentDefinition& d) {
    Json ports = Json::array();
    for (const auto& p : d.ports) ports.push_back({{"id",p.id},{"name",p.name},{"domain",domainName(p.domain)},{"kind",portKindName(p.kind)},{"direction",signalDirectionName(p.direction)},{"signal",dimensionJson(p.signalDimension)},{"effort",dimensionJson(p.effortDimension)},{"flow",dimensionJson(p.flowDimension)}});
    Json parameters = Json::array();
    for (const auto& p : d.parameters) parameters.push_back({{"id",p.id},{"name",p.name},{"dimension",dimensionJson(p.dimension)},{"default",p.defaultSiValue},{"minimum",p.minimumSiValue ? Json(*p.minimumSiValue) : Json(nullptr)},{"maximum",p.maximumSiValue ? Json(*p.maximumSiValue) : Json(nullptr)}});
    return {{"id",d.ref.id},{"version",d.ref.version},{"name",d.name},{"domain",domainName(d.domain)},{"ports",ports},{"parameters",parameters}};
}
Result<ComponentDefinition> readDefinition(const Json& j) {
    try {
        ComponentDefinition d;
        d.ref = {j.at("id").get<std::string>(), j.at("version").get<std::string>()};
        d.name = j.at("name").get<std::string>();
        auto domain = parseDomain(j.at("domain").get<std::string>()); if (!domain) return Result<ComponentDefinition>::failure(domain.error()); d.domain = domain.value();
        if (!j.at("ports").is_array() || j.at("ports").size() > 10000 || !j.at("parameters").is_array() || j.at("parameters").size() > 10000)
            return Result<ComponentDefinition>::failure(makeError(ErrorCode::ResourceLimit, "Definition port or parameter count exceeds limit."));
        for (const auto& p : j.at("ports")) {
            PortDefinition port; port.id=p.at("id").get<std::string>(); port.name=p.at("name").get<std::string>();
            auto pd=parseDomain(p.at("domain").get<std::string>()); auto pk=parsePortKind(p.at("kind").get<std::string>()); auto dir=parseSignalDirection(p.at("direction").get<std::string>());
            auto sig=readDimension(p.at("signal")); auto eff=readDimension(p.at("effort")); auto flow=readDimension(p.at("flow"));
            if(!pd) return Result<ComponentDefinition>::failure(pd.error()); if(!pk) return Result<ComponentDefinition>::failure(pk.error()); if(!dir) return Result<ComponentDefinition>::failure(dir.error()); if(!sig) return Result<ComponentDefinition>::failure(sig.error()); if(!eff) return Result<ComponentDefinition>::failure(eff.error()); if(!flow) return Result<ComponentDefinition>::failure(flow.error());
            port.domain=pd.value(); port.kind=pk.value(); port.direction=dir.value(); port.signalDimension=sig.value(); port.effortDimension=eff.value(); port.flowDimension=flow.value(); d.ports.push_back(std::move(port));
        }
        for (const auto& p : j.at("parameters")) {
            auto dim=readDimension(p.at("dimension")); if(!dim) return Result<ComponentDefinition>::failure(dim.error());
            ParameterDefinition parameter{p.at("id").get<std::string>(),p.at("name").get<std::string>(),dim.value(),p.at("default").get<double>(),{}, {}};
            if (!p.at("minimum").is_null()) parameter.minimumSiValue=p.at("minimum").get<double>();
            if (!p.at("maximum").is_null()) parameter.maximumSiValue=p.at("maximum").get<double>();
            d.parameters.push_back(std::move(parameter));
        }
        auto valid=validateDefinition(d); if(!valid) return Result<ComponentDefinition>::failure(valid.error());
        return Result<ComponentDefinition>::success(std::move(d));
    } catch (const std::exception&) { return Result<ComponentDefinition>::failure(makeError(ErrorCode::CorruptData,"Malformed component definition.")); }
}
Json projectJson(const ProjectModel& p) {
    Json definitions=Json::array(), components=Json::array(), connections=Json::array();
    for(const auto& [ref,d]:p.definitions()) { (void)ref; definitions.push_back(definitionJson(d)); }
    for(const auto& [id,c]:p.components()) {
        Json params=Json::object(); for(const auto& [key,q]:c.parameters) params[key]={{"value",q.si_value},{"dimension",dimensionJson(q.dimension)}};
        components.push_back({{"id",id},{"definition",{{"id",c.definition.id},{"version",c.definition.version}}},{"display_name",c.displayName},{"parameters",params}});
    }
    for(const auto& c:p.connections()) connections.push_back({{"first",{{"component",c.first.componentId},{"port",c.first.portId}}},{"second",{{"component",c.second.componentId},{"port",c.second.portId}}}});
    return {{"id",p.id()},{"name",p.name()},{"definitions",definitions},{"components",components},{"connections",connections}};
}
Result<ProjectModel> parseProject(const Json& j) {
    try {
        if(!j.at("definitions").is_array() || j.at("definitions").size()>maxDefinitions || !j.at("components").is_array() || j.at("components").size()>maxComponents || !j.at("connections").is_array() || j.at("connections").size()>maxConnections)
            return Result<ProjectModel>::failure(makeError(ErrorCode::ResourceLimit,"Project object count exceeds the file format limit."));
        ProjectModel p(j.at("id").get<std::string>(),j.at("name").get<std::string>());
        for(const auto& x:j.at("definitions")) { auto d=readDefinition(x); if(!d) return Result<ProjectModel>::failure(d.error()); auto r=p.addDefinition(std::move(d.value())); if(!r) return Result<ProjectModel>::failure(r.error()); }
        for(const auto& x:j.at("components")) {
            ComponentInstance c; c.id=x.at("id").get<std::string>(); c.definition={x.at("definition").at("id").get<std::string>(),x.at("definition").at("version").get<std::string>()}; c.displayName=x.at("display_name").get<std::string>();
            if(!x.at("parameters").is_object()) return Result<ProjectModel>::failure(makeError(ErrorCode::CorruptData,"Component parameters must be an object."));
            for(auto it=x.at("parameters").begin();it!=x.at("parameters").end();++it) { auto dim=readDimension(it.value().at("dimension")); if(!dim) return Result<ProjectModel>::failure(dim.error()); c.parameters.emplace(it.key(),Quantity{it.value().at("value").get<double>(),dim.value()}); }
            auto r=p.addComponent(std::move(c)); if(!r) return Result<ProjectModel>::failure(r.error());
        }
        for(const auto& x:j.at("connections")) { auto r=p.connect({x.at("first").at("component").get<std::string>(),x.at("first").at("port").get<std::string>()},{x.at("second").at("component").get<std::string>(),x.at("second").at("port").get<std::string>()}); if(!r) return Result<ProjectModel>::failure(r.error()); }
        auto valid=p.validate(); if(!valid) return Result<ProjectModel>::failure(valid.error());
        return Result<ProjectModel>::success(std::move(p));
    } catch(const std::exception&) { return Result<ProjectModel>::failure(makeError(ErrorCode::CorruptData,"Malformed project JSON structure.")); }
}
Result<std::string> extract(mz_zip_archive& zip,const char* name) {
    size_t size{}; void* data=mz_zip_reader_extract_file_to_heap(&zip,name,&size,0);
    if(!data) return Result<std::string>::failure(makeError(ErrorCode::CorruptData,"Cannot extract a required archive entry or its checksum is invalid."));
    if(size>maxEntryBytes) { mz_free(data); return Result<std::string>::failure(makeError(ErrorCode::ResourceLimit,"Project entry exceeds size limit.")); }
    std::string result(static_cast<const char*>(data),size); mz_free(data); return Result<std::string>::success(std::move(result));
}
bool writeAtomically(const std::filesystem::path& path,const std::vector<std::uint8_t>& bytes) {
    auto temp=path; temp += ".tmp";
    { std::ofstream out(temp,std::ios::binary|std::ios::trunc); if(!out) return false; out.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size())); out.flush(); if(!out) { out.close(); std::error_code ec; std::filesystem::remove(temp,ec); return false; } }
#ifdef _WIN32
    if(MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) return true;
    std::error_code ec; std::filesystem::remove(temp,ec); return false;
#else
    std::error_code ec; std::filesystem::rename(temp,path,ec); if(!ec) return true; std::filesystem::remove(temp,ec); return false;
#endif
}
} // namespace

Result<void> ProjectStore::save(const ProjectModel& project,const std::filesystem::path& filePath) {
    auto valid=project.validate(); if(!valid) return valid;
    const auto json=projectJson(project).dump(2);
    if(json.size()>maxEntryBytes) return Result<void>::failure(makeError(ErrorCode::ResourceLimit,"Project JSON exceeds size limit."));
    const auto manifest=Json{{"format_id","org.openlab.olsim"},{"format_version",formatVersion},{"project_file","project.json"}}.dump(2);
    mz_zip_archive zip{}; void* archive=nullptr; size_t archiveSize{};
    if(!mz_zip_writer_init_heap(&zip,0,4096)) return Result<void>::failure(ioError("Could not initialize in-memory OLSIM archive."));
    bool ok=mz_zip_writer_add_mem(&zip,"manifest.json",manifest.data(),manifest.size(),MZ_BEST_COMPRESSION)!=0 && mz_zip_writer_add_mem(&zip,"project.json",json.data(),json.size(),MZ_BEST_COMPRESSION)!=0 && mz_zip_writer_finalize_heap_archive(&zip,&archive,&archiveSize)!=0;
    mz_zip_writer_end(&zip);
    if(!ok || !archive) { if(archive) mz_free(archive); return Result<void>::failure(ioError("Could not build OLSIM ZIP archive.")); }
    if(archiveSize>maxArchiveBytes) { mz_free(archive); return Result<void>::failure(makeError(ErrorCode::ResourceLimit,"OLSIM archive exceeds size limit.")); }
    std::vector<std::uint8_t> bytes(static_cast<std::uint8_t*>(archive),static_cast<std::uint8_t*>(archive)+archiveSize); mz_free(archive);
    if(!writeAtomically(filePath,bytes)) return Result<void>::failure(ioError("Could not atomically write OLSIM project."));
    return Result<void>::success();
}

Result<ProjectModel> ProjectStore::load(const std::filesystem::path& filePath) {
    std::error_code ec; const auto size=std::filesystem::file_size(filePath,ec);
    if(ec) return Result<ProjectModel>::failure(ioError("Could not inspect OLSIM project file."));
    if(size==0 || size>maxArchiveBytes) return Result<ProjectModel>::failure(makeError(ErrorCode::ResourceLimit,"OLSIM archive is empty or exceeds size limit."));
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size)); std::ifstream in(filePath,std::ios::binary); if(!in || !in.read(reinterpret_cast<char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()))) return Result<ProjectModel>::failure(ioError("Could not read OLSIM project file."));
    mz_zip_archive zip{}; if(!mz_zip_reader_init_mem(&zip,bytes.data(),bytes.size(),0)) return Result<ProjectModel>::failure(makeError(ErrorCode::CorruptData,"File is not a readable OLSIM ZIP archive."));
    const mz_uint count=mz_zip_reader_get_num_files(&zip); std::set<std::string> names;
    if(count!=2) { mz_zip_reader_end(&zip); return Result<ProjectModel>::failure(makeError(ErrorCode::CorruptData,"OLSIM v1 requires exactly two archive entries.")); }
    bool entriesOk=true;
    for(mz_uint i=0;i<count;++i) { mz_zip_archive_file_stat stat{}; if(!mz_zip_reader_file_stat(&zip,i,&stat) || stat.m_is_directory || stat.m_uncomp_size>maxEntryBytes) { entriesOk=false; break; } names.emplace(stat.m_filename); }
    if(!entriesOk || names!=std::set<std::string>{"manifest.json","project.json"}) { mz_zip_reader_end(&zip); return Result<ProjectModel>::failure(makeError(ErrorCode::CorruptData,"OLSIM contains unexpected, duplicate or oversized entries.")); }
    auto manifestText=extract(zip,"manifest.json"); auto projectText=extract(zip,"project.json"); mz_zip_reader_end(&zip);
    if(!manifestText) return Result<ProjectModel>::failure(manifestText.error()); if(!projectText) return Result<ProjectModel>::failure(projectText.error());
    try {
        const auto manifest=Json::parse(manifestText.value());
        if(manifest.at("format_id")!="org.openlab.olsim" || manifest.at("project_file")!="project.json") return Result<ProjectModel>::failure(makeError(ErrorCode::CorruptData,"Unexpected OLSIM manifest."));
        const auto version=manifest.at("format_version").get<unsigned>();
        if(version!=formatVersion) return Result<ProjectModel>::failure(makeError(ErrorCode::UnsupportedVersion,"This OLSIM format version is not supported; no migration is registered."));
        return parseProject(Json::parse(projectText.value()));
    } catch(const std::exception&) { return Result<ProjectModel>::failure(makeError(ErrorCode::CorruptData,"Malformed OLSIM manifest or JSON.")); }
}

} // namespace openlab::core
