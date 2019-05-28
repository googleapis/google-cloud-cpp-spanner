// This is a demo used to exercise the Spanner API as we're developing it.
// Eventually this should be replaced with tests and samples.

#include "google/cloud/optional.h"
#include "google/cloud/spanner/client.h"
#include "google/cloud/spanner/key.h"
#include "google/cloud/spanner/mutation.h"
#include "google/cloud/spanner/partition.h"
#include "google/cloud/spanner/result_set.h"
#include "google/cloud/spanner/row.h"
#include "google/cloud/spanner/sql_statement.h"
#include "google/cloud/spanner/transaction.h"
#include "google/cloud/spanner/value.h"
#include "google/cloud/status.h"
#include "google/cloud/status_or.h"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

namespace spanner = ::google::cloud::spanner;
using ::google::cloud::Status;
using ::google::cloud::StatusOr;

int main() {
  StatusOr<spanner::Client> sc =
      spanner::MakeClient({{"label_key", "label_val"}});
  if (!sc) {
    return 1;
  }

  std::string const table = "MyTable";
  std::vector<std::string> const columns = {"A", "B", "C", "D", "E"};

  spanner::Transaction tx = spanner::MakeReadOnlyTransaction();

  // Demonstrate serializing and deserializing a Transaction
  std::string data = spanner::SerializeTransaction(tx);
  StatusOr<spanner::Transaction> tx2 = spanner::DeserializeTransaction(data);
  if (!tx2) {
    return 3;
  }
  /* assert(tx == tx2); */
  std::cout << "Using serialized transaction: " << data << "\n";

  StatusOr<spanner::Client> sc2 = spanner::MakeClient(*tx2);

  std::cout << "\n# Using Client::Read()...\n";
  spanner::KeySet keys("index2");
  keys.Add(spanner::Key(1.0, true, "hello"));
  spanner::ResultSet result = sc->Read(tx, table, std::move(keys), columns);

  spanner::ColumnRange cols = result.columns();
  StatusOr<spanner::Column<int64_t>> col_d = cols.get<int64_t>("D");
  StatusOr<spanner::ColumnBase> col_e = cols.get("E");
  if (!col_d || !col_e) {
    std::cerr << "Unexpected column columns\n";
    return -1;
  }

  for (StatusOr<spanner::Row>& row : result.rows()) {
    if (!row) {
      std::cout << "Read failed\n";
      continue;  // Or break? Can the next read succeed?
    }
    google::cloud::optional<int64_t> v;
    v = row->get(*col_d);
    if (v) {
      std::cout << *v;
    } else {
      std::cout << "null";
    }
    std::cout << "\n";

    v = row->get<int64_t>(*col_e);
    (v ? std::cout << *v : std::cout << "null") << "\n";
  }

  std::cout << "\n# Using Client::Read()...\n";

  spanner::KeySet keys2("index2");
  keys2.Add(spanner::Key(1.0, true, "hello"));
  spanner::ResultSet result2 = sc->Read(tx, table, std::move(keys2), columns);
  spanner::ColumnRange cols2 = result2.columns();
  StatusOr<spanner::Column<std::string>> col_singer_id =
      cols2.get<std::string>("SingerId");
  StatusOr<spanner::Column<std::string>> col_album_id =
      cols2.get<std::string>("AlbumId");
  StatusOr<spanner::Column<std::string>> col_album_title =
      cols2.get<std::string>("AlbumTitle");
  if (!col_singer_id || !col_album_id || !col_album_title) {
    std::cerr << "Unexpected column columns\n";
    return -1;
  }

  for (StatusOr<spanner::Row>& row : result.rows()) {
    if (!row) {
      std::cout << "Read failed\n";
      continue;  // Or break? Can the next read succeed?
    }
    std::cout << col_singer_id->name() << ": " << *row->get(*col_singer_id)
              << ", "
              << "AlbumId: " << *row->get(*col_album_id) << ", "
              << "AlbumTitle: " << *row->get(*col_album_title) << "\n";
  }

  spanner::KeySet keys3("index2");
  keys3.Add(spanner::Key(1.0, true, "hello"));
  result = sc->Read(tx, table, std::move(keys3), columns);
  cols = result.columns();
  for (StatusOr<spanner::Row>& row : result.rows()) {
    if (!row) {
      std::cout << "Read failed\n";
      continue;  // Or break? Can the next read succeed?
    }
    for (spanner::ColumnBase const& c : cols) {
      auto x = row->get<int64_t>(c);
    }
    // ...
  }

#if 0
  // Uses Client::Read().  This uses an old Row API - should we delete it?
  std::cout << "\n# Using Client::Read()...\n";
  result = sc->Read(tx, table, std::move(keys), columns);
  for (StatusOr<spanner::Row>& row : result.rows()) {
    if (!row) {
      std::cout << "Read failed\n";
      continue;  // Or break? Can the next read succeed?
    }

    // You can access values via accessors on the Row. You can specify either

    // the column name or the column's index.
    /* assert(row->is<int64_t>("D")); */
    google::cloud::optional<int64_t> d = row->get<int64_t>("D");
    std::cout << "D=" << d.value_or(-1) << "\n";

    /* assert(row->is<int64_t>(3)); */
    d = row->get<int64_t>(3);
    std::cout << "D(index 3)=" << d.value_or(-1) << "\n";

    // Additionally, you can iterate all the Values in a Row.
    std::cout << "Row:\n";
    for (spanner::Value& value : *row) {
      if (value.is<bool>()) {
        std::cout << "BOOL(" << *value.get<bool>() << ")\n";
      } else if (value.is<int64_t>()) {
        std::cout << "INT64(" << value.get<int64_t>().value_or(-1) << ")\n";

      } else if (value.is<double>()) {
        std::cout << "FLOAT64(" << *value.get<double>() << ")\n";
      }
      // ...
    }
  }
#endif

  // Uses Client::ExecuteSql().
  std::cout << "\n# Using Client::ExecuteSql()...\n";
  spanner::SqlStatement sql(
      "select * from Mytable where id > @msg_id and name like @name",
      {{"msg_id", spanner::Value(int64_t{123})},
       {"name", spanner::Value(std::string("sally"))}});
  result = sc->ExecuteSql(tx, sql);
  for (StatusOr<spanner::Row>& row : result.rows()) {
    if (!row) {
      std::cout << "Read failed\n";
      continue;  // Or break? Can the next read succeed?
    }
    // ...
  }

  Status s = sc->Rollback(tx);
  // assert(s)

  // Commit Mutations
  std::cout << "\n# Using Client::Commit()...\n";
  spanner::Transaction rw_tx = spanner::MakeReadWriteTransaction();

  spanner::KeySet delete_keys("index2");
  delete_keys.Add(spanner::Key(int64_t{0}, true, "hello"));

// TODO(salty) all these 'if 0' sections were from the old Row/Cell world;
// they need to be updated to work with Row/Column/Value
#if 0
  // TODO(salty) No longer relevant?
  std::vector<spanner::Row> insert_rows;
  spanner::Row row;
  row.Addspanner::Value(spanner::Value("col1", int64_t{9}));
  row.Addspanner::Value(spanner::Value("col2", true));
  row.Addspanner::Value(spanner::Value("col3", "data4"));
  insert_rows.push_back(row);
  row.Addspanner::Value(spanner::Value("col1", int64_t{4}));
  row.Addspanner::Value(spanner::Value("col2", true));
  row.Addspanner::Value(spanner::Value("col3", "data9"));
  insert_rows.push_back(row);
  row.Addspanner::Value(spanner::Value("col1", int64_t{0}));
  row.Addspanner::Value(spanner::Value("col2", false));
  row.Addspanner::Value(spanner::Value("col3", "data0"));
  insert_rows.push_back(row);
#endif

  std::vector<spanner::Mutation> mutations = {
#if 0
      spanner::Mutation::Insert("MyTable", insert_rows),
      // table and vector of rows, where columns must match
      spanner::Mutation::InsertOrUpdate(
          "MyTable",
          {
              spanner::Row({spanner::Value("col1", int64_t{3}),
                            spanner::Value("col2", true),
                            spanner::Value("col3", "data")}),
              spanner::Row({spanner::Value("col1", int64_t{8}),
                            spanner::Value("col2", false),
                            spanner::Value("col3", "d2")}),
              spanner::Row({spanner::Value("col1", int64_t{42}),
                            spanner::Value("col2", false),
                            spanner::Value("col3", "d3")}),
          }),
      // table, then columns, then values only.
      spanner::Mutation::Update(
          "MyTable", {"col1", "col2", "col3"},
          {
              {spanner::Value(int64_t{3}), spanner::Value(true),
               spanner::Value("data")},
              {spanner::Value(int64_t{8}), spanner::Value(false),
               spanner::Value("d2")},
              {spanner::Value(int64_t{42}), spanner::Value(false),
               spanner::Value("d3")},
          }),
      // TODO(salty) it would be nice to omit the spanner::Value() constructor
      // calls above and just be able to write this, but that's not currently
      // possible without making the Value constructor non-explicit.
      // Maybe the forthcoming Row() implementation could do this.
      spanner::Mutation::Update("MyTable", {"col1", "col2", "col3"},
                                {
                                    {int64_t{3}, true, "data"},
                                    {int64_t{8}, false, "d2"},
                                    {int64_t{42}, false, "d3"},
                                }),
#endif
    spanner::Mutation::Delete("MyTable", delete_keys),
    spanner::Mutation::Delete(
        "MyTable", spanner::KeySet("index2", {spanner::Key(std::int64_t{0},
                                                           true, "hello")})),
  };

  auto rw_result = sc->Commit(rw_tx, mutations);
  if (rw_result) {
    std::cout << std::chrono::system_clock::to_time_t(*rw_result) << "\n";
  }  // else error
}
