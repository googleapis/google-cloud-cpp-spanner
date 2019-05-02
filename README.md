# Google Cloud Spanner C++ Client Library

Cloud Spanner is the only enterprise-grade, globally-distributed, and
strongly consistent database service built for the cloud specifically
to combine the benefits of relational database structure with
non-relational horizontal scale. This combination delivers
high-performance transactions and strong consistency across rows,
regions, and continents with an industry-leading 99.999% availability
SLA, no planned downtime, and enterprise-grade security. Cloud Spanner
revolutionizes database administration and management and makes
application development more efficient.

This directory contains the implementation of the Cloud Spanner C++
client library.

## Status

This library is under active development, it is known to be incomplete,
and is subject to change without notice. We do not recommend that you use
this library for experimental code, nor for production workloads.

## Documentation

* Please consult the [Cloud Spanner website][cloud-spanner-docs] for
  general documentation on Cloud Spanner features, APIS, other client
  libraries, etc.

[cloud-spanner-docs]: https://cloud.google.com/spanner/docs/

## Contributing changes

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for details on how to contribute to
this project, including how to build and test your changes as well as how to
properly format your code.

## Licensing

Apache 2.0; see [`LICENSE`](LICENSE) for details.

## Release Notes

### v0.1.x - TBD

## Versioning

Please note that the Google Cloud C++ client libraries do **not** follow
[Semantic Versioning](http://semver.org/).

**GA**: Libraries defined at a GA quality level are expected to be stable and
any backwards-incompatible changes will be noted in the documentation. Major
changes to the API will signaled by changing major version number
(e.g. 1.x.y -> 2.0.0).

**Beta**: Libraries defined at a Beta quality level are expected to be mostly
stable and we're working towards their release candidate. We will address issues
and requests with a higher priority.

**Alpha**: Libraries defined at an Alpha quality level are still a
work-in-progress and are more likely to get backwards-incompatible updates.
Additionally, it's possible for Alpha libraries to get deprecated and deleted
before ever being promoted to Beta or GA.

