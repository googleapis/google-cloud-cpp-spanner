#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_SQL_STATEMENT_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_SQL_STATEMENT_H_

#include "google/cloud/spanner/version.h"
#include <map>
#include <string>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

// Represents an SQL statement with optional parameters. Parameter placeholders
// may be specified in the sql string using `@` followed by the parameter name.
// ... follow Spanner's docs about this.
class SqlStatement {
 public:
  // XXX: Can/should we re-use Value here? It could be made more generic so
  // that it's column attribute is just a generic "name", not necessarily a
  // column?
  using param_type = std::map<std::string, Value>;
  explicit SqlStatement(std::string sql, param_type params)
      : sql_(std::move(sql)), params_(std::move(params)) {}

  std::string sql() const { return sql_; }
  param_type params() const { return params_; }

 private:
  std::string sql_;
  param_type params_;
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_SQL_STATEMENT_H_
