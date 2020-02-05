# Changelog

## v0.8.x - TBD

## v0.7.x - 2020-02

* **Breaking Changes**
  * fix!: avoid single-arg forwarding c'tor (#1233)
  * chore!: prep cleanup for enabling clang-tidy in headers (#1222)

* **Other Changes**
  * docs: add summary of result classes (#1243)
  * feat: const qualify ConnectionOptions::background_threads_factory() (#1240)
  * doc: document how to run samples using Bazel (#1228)
  * feat: implement query and update timestamp samples (#1223)
  * feat: add CommitTimestamp support (#1225)
  * doc: add skeletons for *_with_timestamp_column (#1220)
  * doc: add spanner_create_user_defined_struct example (#1214)
  * style: cleanup use of `noexcept`, `explicit` and `= default` (#1209)
  * doc: exclude benchmarks for doxygen docs (#1208)
  * docs: specify how Commit(mutator, ...) handles exceptions (#1198)
  * feat: use 1 client with multiple channels for the benchmark (#1194)
  * fix: remove unnecessary dependency on bigtable (#1195)
  * feat: support truncation of strings/bytes fields in protobuf logging (#1192)
  * feat: a small script to plot benchmark results (#1189)
  * fix: complete test coverage for g:c:spanner::Timestamp (#1190)
  * fix: only check for per-thread rusage in the benchmark that requires it (#1186)
  * docs: Tighten up some documentation for g:c:spanner::Client::Commit() (#1173)
  * feat: remove internal::TimestampFromCounts() (#1183)

## v0.6.x - 2020-01

* **Breaking Changes**
  * feat!: remove Timestamp::Min() and Timestamp::Max() (#1182)

* **Other Changes**
  * feat: allow re-using a database across benchmark runs (#1174)
  * fix: clean up / fix ClientCount logic (#1175)

## v0.5.x - 2020-01

* **Breaking Changes**
  * fix!: support full TIMESTAMP range in g::c::spanner::Timestamp (#1166)
  * refactor!: make `SessionPoolOptions` a class (#1130)

* **Other Changes**
  * fix: avoid deadlock waiting for Session allocation (#1170)
  * bug: number of channels was inadvertently limited to 1 (#1169)
  * bug: deflake rollback integration test (#1164)
  * feat: add optimistic concurrency control loop for IAM (#1162)
  * feat: add build to check API breakage (#1155)
  * feat: add CMake option to disable -Werror/-WX (#1150)
  * feat: commit begins transaction if not already begun (#1143)
  * feat: delete bad sessions from pool (#1142)
  * feat: handle session-not-found in all connection_impl methods (#1141)
  * bug: set "sticky" I/O format flags before forking threads (#1128)

## v0.4.x - 2019-12

* **Breaking Changes**
    * feat!: replaces PartitionOptions proto with struct (#1035)
    * refactor!: drop support for compile-time preprocessor config (#1086)

* **Other Changes**
    * feat: add `SessionPoolOptions` with labels to `MakeConnection()` (#1109)
    * fix: tell `clang-format` to leave doxygen `@copydoc` directives alone (#1122)
    * feat: allow users to configure retry policies (#1111)
    * feat: optimize passing `Bytes` to `Value` (#1104)
    * feat: change `GetCurrentRow` to `GetSingularRow` (#1092)
    * feat: update the pool options to match the design (#1089)
    * feat: support multiple channels in `SessionPool` (#1063)
    * bug: fix year formatting in `spanner::Date` (#1085)
    * fix: execute each rerun of an aborted transaction in the same session (#1079)
    * feat: add function to get a single row from a range (#1074)
    * feat: add some plumbing to enable multiple channels (#1050)
    * feat: attempt to speed up hot loop in `NextRow` (#1052)
    * feat: associate `SpannerStub` with a `Session` (#1041)
    * add and update several examples, benchmarks, and tests.

## v0.3.x - 2019-11

* **Breaking Changes**
    * feat!: class templates removed from keys.h (#936)
    * feat!: change result return types (#942)
    * refactor!: replaced Row<...> with std::tuple<...> (#967)
    * feat!: support for select-star queries (#976)
    * feat!: replace RunTransaction() with Client::Commit(functor) (#975)
    * chore!: renamed QueryResult to RowStream (#978)
    * chore!: renamed ExecuteSqlParams to SqlParams (#986)
    * cleanup: depend on google-cloud-cpp-common (#940)

* feat: configurable strategy for background threads (#955)
* feat: add Profile and Analyze methods (#961)
* feat: adds efficient move to support to Value:get<string>() (#980)
* feat: add efficient move support to mutation builder temporaries (#989)
* bug: only install the required headers (#993)
* bug: install the headers for mocking (#997)

## v0.2.x - 2019-10
* **Breaking Changes**
  * refactor `Read` to return `ReadResult`; remove `ResultSet` (#935)
  * removed `Row<>` from mutations API (#938). Removes the `AddRow(Row<Ts...>)`
    member function on the `WriteMutation` API. In place of this method there
    is now an `AddRow(std::vector<Value>)` method.
  * Change `Value::Bytes` to `google::cloud::spanner::Bytes` (#920)
  * implement `CreateInstanceRequestBuilder` (#933). Changed the function
    signature of `InstanceAdminClient::CreateInstance()`.
  * Replace `ExecuteSql` with `ExecuteQuery` and `ExecuteDml` (#927)
  * Changed `RowParser` to require a `Row<Ts...>` template param (#653).
    `ResultSet::Rows` used to be a variadic template that took the individual
     C++ types for each row. With this change that function is now a template
     with one parameter, which must be a `Row<...>` type.
  * Implements `Database` in terms of `Instance` (#652). This PR removes
    renames some accessors like `InstanceId` -> `instance_id` due to their
    trivial nature now (style guide). It also removes some methods like
    `Database::ParentName()`, which is now replaced by `Database::instance()`.
  * Fixes inconsistent naming of the Batch DML params struct. (#650). This
    struct has been renamed, so any code using this struct will need to be
    updated.

* **Feature changes**
  * implement `InstanceAdminClient` methods `CreateInstance`, `UpdateInstance`,
    `DeleteInstance`, `ListInstanceConfigs`, `GetInstanceConfig`,
     `SetIamPolicy`.
  * implement `DatabaseAdminClient` methods `TestIamPermissions`,
    `SetIamPolicy`, `GetIamPolicy`.
  * implement retries for `Commit()`, `PartitionRead`, `PartitionQuery`,
    `ExecuteBatchDml`, `CreateSession`, `Rollback`.
  * implement `PartialResultSetRead` with resumes (#693)
  * use separate policies for retry vs. rerun (#667)
  * implement `DatabaseAdminConnection` (#638)
  * implement overloads on `UpdateInstanceRequestBuilder` (#921)
  * implement metadata decorator for `InstanceAdminStub`. (#678)
  * implement logging wrapper for `InstanceAdminStub`. (#676)
  * support install components for the library (#659)

* **Bug fixes**
  * fix runtime install directory (#658)
  * give sample programs a long timeout time. (#622)
  * use RunTransaction for read write transaction sample (#654)

## v0.1.x - 2019-09

* This is the initial Alpha release of the library.
* While this version is not recommended for production workloads it is stable
  enough to use in experimental code. We welcome feedback about the library
  through [GitHub issues][GitHub-new-issue].
* The API is expected to undergo incompatible changes before Beta and GA.
* The library supports all the operations to read and write data into Cloud
  Spanner.
* The library supports some administrative operations such as creating,
  updating, and dropping databases.

[GitHub-new-issue]: https://github.com/googleapis/google-cloud-cpp-spanner/issues/new

