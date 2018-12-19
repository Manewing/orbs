#ifndef CONFIG_READER_HH
#define CONFIG_READER_HH

#include <stddef.h>

/// A configuration element that can be used to configure a given variable. The
/// element consists of a name for identifcation, documentation, type as well as
/// a pointer to the to variable it is configuring.
typedef struct {

  /// The name of the element, used for identifcation
  const char *name;

  /// The type of the element, corresponds to format parameter of sscanf
  const char *type;

  /// A documentation string of the format parameter
  const char *doc;

  /// Pointer to the variable that is configured by this element
  void *ptr;

} config_elem_t;

/// Creates a configuration element from a sub-object of a struct.
/// @param cfg_t  - The type of the struct used to determine the subobj. offset
/// @param cfg    - The actual struct object
/// @param elem_t - Type of the sub-object, see config_elem_t
/// @param elem   - Name of the element in the struct
/// @param doc    - Documentation string of the element
#define CONFIG_ELEM(cfg_t, cfg, elem_t, elem, doc)                             \
  { #cfg "." #elem, elem_t, doc, (char *)&cfg + offsetof(cfg_t, elem) }

/// A configuration reader consisting of the elements it can configure.
typedef struct {

  /// Elements that are configured by this reader
  config_elem_t const *elems;

  /// Number of elements in the reader
  unsigned int elem_count;

} config_reader_t;

/// Returns the element with given name from the configuration reader or NULL
/// if not found.
/// @param[in] cfg  - The configuration reader to search in
/// @param[in] name - Name of the element to search for
/// @return Pointer to the element or NULL
config_elem_t const *get_config_elem(config_reader_t const *cfg,
                                     const char *name);

/// Reads the given value into the element specified by name of the reader.
/// @param[in] cfg   - The configuration reader
/// @param[in] name  - Name of the element to configure
/// @param[in] value - The value to set the element to
/// @return 0 if successful, -1 if element not found or if the value could
///   not be parsed
int read_config_value(config_reader_t const *cfg, const char *name,
                      const char *value);

/// Reads the given line and configures the element. Expected line format:
///
///   line: "element_name = value"
///
/// @param[in] cfg  - The configuration reader
/// @param[in] line - The line to read
/// @return 0 if successful, -1 if line could not be parsed
int read_config_line(config_reader_t const *cfg, const char *line);

/// Reads the given configuration file and configures the specified elements.
/// Note that empty lines and lines that start with '#' are ignored.
/// @param[in] cfg  - The configuration reader
/// @param[in] file - The file to open and read
/// @return 0 if successful, -1 if file could not be opened or parsed
int read_config_file(config_reader_t const *cfg, const char *file);

/// Prints the documentation of the configuration
/// @param[in] cfg  - The configuration reader
void print_config_options(config_reader_t const *cfg);

#endif // #ifndef CONFIG_READER_HH
