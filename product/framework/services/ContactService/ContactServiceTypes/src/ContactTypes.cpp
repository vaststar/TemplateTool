// ContactServiceTypes holds the service's outward data structures. Those are
// currently pure-virtual entity interfaces (IContactEntities.h) which are a
// compile-time contract and emit no linkable symbols, so this exported anchor
// gives the shared library a symbol and an import library. Replace it with a
// real exported type once one gains an out-of-line body.
#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

#include <ucf/Services/ContactService/IContactEntities.h>

namespace ucf::service::model {

SERVICE_EXPORT void contactTypesAnchor() {}

}
