# MOT Internal CLI

This document describes the `cli_mot` executable which is called by the `mot` shell function. It is not typically called directly by users.

## Usage

```
cli_mot <mode> [OPTIONS]
```

Mode is either `-d` (directory) or `-i` (interactive).

### Directory Mode

Directory mode is used to ask cli_mot to provide a directory based on an identifier defined in the user's MOT profile. In this mode, cli_mot's output is expected to be buffered by the calling shell function.

Usage:

```
cli_mot -d [namespace:]<identifier>
```

The namespace is optional. Its value defaults to the environment variable `MOT_NS`. If the namespace is not specified and `MOT_NS` is not set, cli_mot will return an error.

Examples:

```
cli_mot -d wsl:mydir
cli_mot -d mydir
```

Return Codes:

- 0 indicates the directory was found. The directory path will be written to stdout with no terminating new line characters.
- 1 indicates an error. An error message may be written to stdout and/or stderr and should be displayed to the user.
- 2 indicates `cli_mot` should be called again with the same options in interactive mode instead.

Other return codes are undefined. An error message should be displayed to the user.

### Interactive Mode

Interactive mode is used for all other CLI actions. In this mode, cli_mot is expected to have direct access to the console (stdin/out/err) and may prompt the user for input if necessary.

Usage:

```
cli_mod -d <MOT_ARGS>
```

`<MOT_ARGS>` are all of the various arguments which are accepted by the `mot` shell function.

Return Codes:

- 0 indicates the command succeeded.
- 3 indicates `cli_mot` should be called again with the same options in directory mode.
- All other return codes indicate an error.
