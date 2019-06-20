# Doxygen Documentation Style Guide

This document describes how to use Doxygen in this project. The contents are
intended as guidelines for developers in the project, they are not hard rules.

1. All types, functions, macros, namespaces in the public API get documentation,
   even "obvious" classes and functions.

1. Prefer `///` for types or functions that just have a single line of
   documentation. For example:
    ```C++
    /// This function computes an approximate value for Pi
    double ComputePi() { return 3; }
    ```

1. For types or functions that get a long comment start with: (a) a one sentence
   description of that the thing is or does, then (b) the rest of the
   documentation. In other words, we use `JAVADOC_AUTOBRIEF=yes`:
    ```C++
    /**
     * This function computes an approximate value for Pi
     *
     * Get a value of Pi adequate for our needs, it may not be super
     * accurate, but it is fast.
     */
    double ComputePi() { return 3; }
    ```

1. We use `/** ... */` for long comments, we do not use `/*! ... */`.

1. Use `@command` in preference `\command` for commands, e.g., `@param` or
   `@return` instead of `\param` or `\return`.

1. For functions, if you document one parameter you need to document all of
   them.

1. When documenting functions add the `@return` section if non-obvious.

1. Large important classes get large documentation, including links to the
   relevant docs in `cloud.google.com`, but  add those late in the development
   cycle.

1. Using Doxygen for private members is entirely optional, we do not extract
   this documentation, but it is easier to use the same style for all comments.

## Additional Information

* The Doxygen website has detailed information on how to format
  [docblocks][docblocks-link].

[docblocks-link]: http://www.doxygen.nl/manual/docblocks.html
