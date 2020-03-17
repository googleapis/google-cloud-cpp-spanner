# Some rules around Spanner resources

We're running integration tests and samples on every PR. We've created
pre-defined Spanner instances for speeding up the tests. Also some of our tests
need to create a new Spanner instance. There are certain rules on instance names
and InstanceConfig for those instances.

Pre-defined Spanner instances have instance ids prefixed with `test-instance-n`
where `n` is just an index number, and they're in `us-central` or `us-east`.

Temporary Spanner instances have instance ids prefixed with
`temporary-instance-YYY-MM-DD` and they're in `us-west*`.
