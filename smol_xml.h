#ifndef SMOL_XML_H
#define SMOL_XML_H

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#ifndef SMOL_UTILS_H
#if defined(_WIN32)
#	ifndef SMOL_PLATFORM_WINDOWS
#		define SMOL_PLATFORM_WINDOWS
#	endif 
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#elif defined(__linux__) 
#	ifndef SMOL_PLATFORM_LINUX
#		define SMOL_PLATFORM_LINUX
#	endif 
# 	define _XOPEN_SOURCE 500
#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <sys/time.h>
#	include <time.h>
#	include <fcntl.h>
#	include <dirent.h>
#elif defined(__APPLE__)
#	define SMOL_PLATFORM_MAC_OS
//TODO:
#	error Mac OS backend not implemented yet!
#elif defined(__EMSCRIPTEN__)
#	ifndef SMOL_PLATFORM_EMSCRIPTEN
#		define SMOL_PLATFORM_EMSCRIPTEN
#	endif 
#	include <emscripten.h>
#endif 

#if defined(SMOL_PLATFORM_WINDOWS)
#	ifndef SMOL_BREAKPOINT
#		ifdef _MSC_VER
#			define SMOL_BREAKPOINT() __debugbreak() //MSVC uses this
#		else
#			define SMOL_BREAKPOINT() DebugBreak()
#		endif 
#	endif 
#elif defined(SMOL_PLATFORM_WEB)
#	define SMOL_BREAKPOINT() EM_ASM({ debugger; })
#elif defined(SMOL_PLATFORM_LINUX)
#	ifndef SMOL_BREAKPOINT
#		define SMOL_BREAKPOINT() raise(SIGTRAP)
#	endif
#endif 

#ifndef SMOL_SYMBOLIFY
#define SMOL_SYMBOLIFY(x) #x
#endif 

#ifndef SMOL_STRINGIFY
#define SMOL_STRINGIFY(x) SMOL_SYMBOLIFY(x)
#endif 

#ifndef SMOL_ASSERT
#define SMOL_ASSERT(condition) \
	if(!(condition)) \
		puts(\
			"SMOL FRAME ASSERTION FAILED!\n" \
			"CONDITION: " #condition "\n" \
			"IN FILE: '" __FILE__ "'\n" \
			"ON LINE: " SMOL_STRINGIFY(__LINE__) "\n" \
		), \
		SMOL_BREAKPOINT()
#endif 

#ifndef SMOL_ALLOC
#define SMOL_ALLOC( size ) malloc(size)
#endif 

#ifndef SMOL_FREE
#define SMOL_FREE( ptr ) free(ptr)
#endif 

#ifndef SMOL_REALLOC
#define SMOL_REALLOC( old_ptr, new_size ) realloc(old_ptr, new_size)
#endif 

/* --------------------------------------- */
/*  Dynamic Array (std::vector) like stuff */
/* --------------------------------------- */

//This macro can be used to define a vector type, or declare a local variable: smol_vector(int) int_vec;
#define smol_vector(type) \
struct { \
	int allocation;\
	int count; \
	type* data; \
}

//smol_vector_init - Init a vector
//Arguments:
// - vec -- The vector to be initialized
// - init_alloc -- The initial allocation of the vector
#define smol_vector_init(vec, init_alloc) { \
	(vec)->allocation = init_alloc; \
	(vec)->count = 0; \
	(vec)->data = SMOL_ALLOC(sizeof(*((vec)->data))*init_alloc); \
} (void)0

//smol_vector_push - Pushes an object into a vector
//Arguments:
// - vec -- The vector to be appended 
// - value -- The element to be added
#define smol_vector_push(vec, value) { \
	if((vec)->count >= (vec)->allocation) { \
		(vec)->allocation *= 2; \
		(vec)->data = SMOL_REALLOC((vec)->data, sizeof(*((vec)->data)) * (vec)->allocation ); \
	} \
	SMOL_ASSERT((vec)->data); \
	(vec)->data[(vec)->count++] = value; \
} (void)0

//smol_vector_iterate - Iterates over vector elements
//Arguments: 
// - vec - The vector to be iterated over
// - it - Variable name for the iterator
//Returns: type* - Containing the buffer pointer to be indexed in
#define smol_vector_iterate(vec, it) \
	(vec)->data; \
	SMOL_ASSERT((vec)->data); \
	for(int it = 0; it < (vec)->count; it++)

//smol_vector_each - Iterates over each element
//Arguments:
// - vec - The vector to be iterated over
// - element_type - A type of individual element in the vector (MSVC doesn't have __typeof__ *sigh*)
// - it - Iterator variable name
#define smol_vector_each(vec, element_type, it) \
	for(element_type* it = (vec)->data; it != &(vec)->data[(vec)->count]; it++) 

//smol_vector_clear - Clears the vector
//Arguments: 
// - vec -- The vector to be cleared
#define smol_vector_clear(vec) { \
	SMOL_ASSERT((vec)->data); \
	((vec)->count = 0); \
} (void)0

//smol_vector_remove - Removes an element from the vector, by overwriting it with the last element, and decreasing vector size
//Arguments: 
// - vec -- The vector to remove element from
// - element - The element index to be removed
#define smol_vector_remove(vec, element) { \
	SMOL_ASSERT((vec)->data); \
	(vec)->data[element] = (vec)->data[--(vec)->count]; \
} (void)0

//smol_vector_count - "Returns" the number of elements in the vector
//Arguments:
// - vec -- The vector you want count of
//Returns int - containing the number of elements in the vector
#define smol_vector_count(vec) \
	((vec)->count)

//smol_vector_data - "Returns" the data buffer of the vector
// - vector -- The span you want the data of
//Returns type* - containing the pointer to the vector data
#define smol_vector_data(vec) \
	((vec)->data)

//smol_span_at - "Returns" an element of the vector. NOT BOUNDS CHECKED.
// - vec -- The vector you want the element of
// - index -- The index of the element
//Returns type - containing the element at index
#define smol_vector_at(vec, index) \
	((vec)->data[index])

//Frees a vector, and sets it's allocation and count to zero
#define smol_vector_free(vec) { \
	SMOL_FREE((void*)(vec)->data); \
	(vec)->count = 0; \
	(vec)->allocation = 0; \
} (void)0

#endif

#define SMOL_XML_INVALID_ID (size_t)(-1)

typedef smol_vector(size_t) smol_id_vector_t;
typedef smol_vector(char) smol_char_vector_t;

typedef struct _smol_xml_attr_t {
	char* name;
	char* value;
} smol_xml_attr_t;

typedef smol_vector(smol_xml_attr_t) smol_xml_attr_vector_t;

typedef struct _smol_xml_node_t {
	size_t id;

	char* tag;

	smol_xml_attr_vector_t attributes;
	char* body;

	size_t parent;
	smol_id_vector_t children;
} smol_xml_node_t;

typedef smol_vector(smol_xml_node_t) smol_xml_node_vector_t;
typedef smol_vector(smol_xml_node_t*) smol_xml_nodeptr_vector_t;

typedef struct _smol_xml_t {
	smol_xml_node_vector_t nodes;
	smol_xml_attr_vector_t header_attributes;
} smol_xml_t;

typedef int (*smol_xml_attr_matcher_cb)(const char*);

smol_xml_t smol_xml_parse(const char* xml);
void smol_xml_free(smol_xml_t* xml);

smol_xml_node_t* smol_xml_get(smol_xml_t* xml, size_t id);
void smol_xml_find_by_tag(smol_xml_t* xml, const char* tag, smol_xml_nodeptr_vector_t* out);
void smol_xml_find_by_attr(smol_xml_t* xml, const char* attr_name, const char* attr_value, smol_xml_nodeptr_vector_t* out);
void smol_xml_find_by_attr_matcher(smol_xml_t* xml, const char* attr_name, smol_xml_attr_matcher_cb matcher, smol_xml_nodeptr_vector_t* out);
smol_xml_attr_t* smol_xml_get_header_attr(smol_xml_t* xml, const char* attr_name);

typedef int (*read_test_cb_t)(char);

typedef struct _smol_scanner_t {
	char* buf;
	size_t ptr, len;
} smol_scanner_t;

void smol_scanner_init(smol_scanner_t* scanner, const char* data);
void smol_scanner_skip(smol_scanner_t* scanner);
char smol_scanner_peek(smol_scanner_t* scanner, int offset);
int smol_scanner_has_ahead(smol_scanner_t* scanner, const char* test);
char* smol_scanner_read(smol_scanner_t* scanner, read_test_cb_t test);
void smol_scanner_skip_spaces(smol_scanner_t* scanner);

#ifdef SMOL_XML_IMPLEMENTATION

#define _SMOL_TEST_CB(name, cond) int _smol_test__##name(char c) { return cond; }

_SMOL_TEST_CB(tag_name, c != '>' && c != ' ' && c != '/')
_SMOL_TEST_CB(id, isalnum(c) || c == '_' || c == ':')
_SMOL_TEST_CB(string, c != '"')
_SMOL_TEST_CB(cdata, c != ']')
_SMOL_TEST_CB(start_tag, c != '<')
_SMOL_TEST_CB(end_tag, c != '>')
_SMOL_TEST_CB(space, c != ' ')

void smol_scanner_init(smol_scanner_t* scanner, const char* data) {
	scanner->ptr = 0;
	scanner->len = strlen(data);
	scanner->buf = data;
}

void smol_scanner_skip(smol_scanner_t* scanner) {
	scanner->ptr++;
}

char smol_scanner_peek(smol_scanner_t* scanner, int offset) {
	return scanner->buf[scanner->ptr + offset];
}

int smol_scanner_has_ahead(smol_scanner_t* scanner, const char* test) {
	int res = 1;
	size_t offset = 0;
	while (test[offset]) {
		if (test[offset] != smol_scanner_peek(scanner, offset)) {
			res = 0;
			break;
		}
		offset++;
	}
	return res;
}

char* smol_scanner_read(smol_scanner_t* scanner, read_test_cb_t test) {
	smol_vector(char) buff;
	smol_vector_init(&buff, 1024);
	while (scanner->buf[scanner->ptr] != NULL && test(scanner->buf[scanner->ptr])) {
		smol_vector_push(&buff, scanner->buf[scanner->ptr]);
		scanner->ptr++;
	}
	smol_vector_push(&buff, '\0');
	return smol_vector_data(&buff);
}

void smol_scanner_skip_spaces(smol_scanner_t* scanner) {
	while (scanner->buf[scanner->ptr] != NULL && isspace(scanner->buf[scanner->ptr])) {
		scanner->ptr++;
	}
}

int _smol_value__is_number(const char* str) {
	size_t len = strlen(str);
	int is_num = 1;
	for (size_t i = 0; i < len; i++) {
		if (!isdigit(str[i])) {
			if (str[i] == '.') continue;
			is_num = 0;
		}
	}
	return is_num;
}

smol_xml_attr_t _smol_xml__parse_attr(smol_scanner_t* scan) {
	smol_xml_attr_t attr = { 0 };

	smol_scanner_skip_spaces(scan);
	assert(_smol_test__id(smol_scanner_peek(scan, 0)) && "Invalid attribute name.");

	attr.name = smol_scanner_read(scan, &_smol_test__id);
	smol_scanner_skip_spaces(scan);

	// equal symbol (optional)
	if (smol_scanner_peek(scan, 0) == '=') {
		smol_scanner_skip(scan); // remove =
		smol_scanner_skip_spaces(scan);

		assert(smol_scanner_peek(scan, 0) == '"' && "Invalid attribute value.");
		smol_scanner_skip(scan); // remove "

		attr.value = smol_scanner_read(scan, &_smol_test__string);
		smol_scanner_skip(scan); // remove "
	}

	smol_scanner_skip_spaces(scan);

	return attr;
}

// parses a single node
void _smol_xml__parse_node(size_t id, size_t parent, smol_scanner_t* scan, smol_xml_node_vector_t* out) {
	smol_scanner_skip_spaces(scan);

	if (smol_scanner_has_ahead(scan, "<!--")) { // comment
		while (!smol_scanner_has_ahead(scan, "-->")) {
			smol_scanner_skip(scan);
		}
		smol_scanner_skip(scan);
		smol_scanner_skip(scan);
		smol_scanner_skip(scan);
		return;
	}

	// validate
	if (smol_scanner_peek(scan, 0) != '<') return;
	smol_scanner_skip(scan); // remove <

	smol_xml_node_t node = { 0 };
	node.id = id;
	node.body = NULL;
	node.parent = parent;
	smol_vector_init(&node.attributes, 16);
	smol_vector_init(&node.children, 1024);

	// extract tag name
	node.tag = smol_scanner_read(scan, &_smol_test__tag_name);
	smol_scanner_skip_spaces(scan);

	// attributes
	if (isalpha(smol_scanner_peek(scan, 0))) {
		while (smol_scanner_peek(scan, 0) != '>' && !smol_scanner_has_ahead(scan, "/>")) {
			smol_xml_attr_t attr = _smol_xml__parse_attr(scan);
			smol_vector_push(&node.attributes, attr);
		}
		smol_scanner_skip_spaces(scan);
	}

	if (smol_scanner_has_ahead(scan, "/>")) { // oneliner: <tag />
		smol_scanner_skip(scan);
		smol_scanner_skip(scan);
		smol_vector_push(out, node);
		return;
	}

	assert(smol_scanner_peek(scan, 0) == '>' && "Expected tag end.");
	smol_scanner_skip(scan);
	smol_scanner_skip_spaces(scan);

	// body
	if (smol_scanner_has_ahead(scan, "<![CDATA[")) { // <![CDATA[
		for (size_t i = 0; i < 9; i++) smol_scanner_skip(scan); // <![CDATA[
		smol_scanner_skip_spaces(scan);
		node.body = smol_scanner_read(scan, &_smol_test__cdata);
		for (size_t i = 0; i < 3; i++) smol_scanner_skip(scan); // ]]>
		smol_scanner_skip_spaces(scan);
	}
	else if (smol_scanner_peek(scan, 0) != '<') { // regular string
		smol_scanner_skip_spaces(scan);
		node.body = smol_scanner_read(scan, &_smol_test__start_tag);
	}

	size_t id_off = 1;
	while (!smol_scanner_has_ahead(scan, "</")) {
		// children
		size_t new_id = smol_vector_count(out) + id_off;
		_smol_xml__parse_node(new_id, id, scan, out);
		smol_scanner_skip_spaces(scan);

		smol_vector_push(&node.children, new_id);

		id_off++;
	}

	// closing tag validation
	assert(smol_scanner_peek(scan, 0) == '<' && "Invalid closing tag.");
	smol_scanner_skip(scan);

	assert(smol_scanner_peek(scan, 0) == '/' && "Invalid closing tag.");
	smol_scanner_skip(scan);

	char* tagClosed = smol_scanner_read(scan, &_smol_test__tag_name);
	assert(strcmp(node.tag, tagClosed) == 0 && "Invalid closing tag.");
	free(tagClosed);

	smol_scanner_skip_spaces(scan);
	assert(smol_scanner_peek(scan, 0) == '>' && "Invalid closing tag.");
	smol_scanner_skip(scan);

	smol_vector_push(out, node);
}

smol_xml_t smol_xml_parse(const char* xml) {
	smol_scanner_t scan; smol_scanner_init(&scan, xml);

	smol_xml_t xml_data = { 0 };
	smol_vector_init(&xml_data.nodes, 128);
	smol_vector_init(&xml_data.header_attributes, 16);

	smol_scanner_skip_spaces(&scan);
	if (smol_scanner_has_ahead(&scan, "<?")) { // xml header? skip.
		smol_scanner_skip(&scan);
		smol_scanner_skip(&scan);
		smol_scanner_skip_spaces(&scan);
		free(smol_scanner_read(&scan, &_smol_test__space)); // skip any tag here typically "xml"

		while (!smol_scanner_has_ahead(&scan, "?>")) {
			smol_xml_attr_t attr = _smol_xml__parse_attr(&scan);
			smol_vector_push(&xml_data.header_attributes, attr);
		}
		smol_scanner_skip(&scan);
		smol_scanner_skip(&scan);
		smol_scanner_skip_spaces(&scan);
	}
	
	_smol_xml__parse_node(0, SMOL_XML_INVALID_ID, &scan, &xml_data.nodes);
	
	return xml_data;
}

smol_xml_node_t* smol_xml_get(smol_xml_t* xml, size_t id) {
	if (id == SMOL_XML_INVALID_ID) return NULL;
	if (id >= smol_vector_count(&xml->nodes)) return NULL;
	return &smol_vector_at(&xml->nodes, id);
}

void smol_xml_find_by_tag(smol_xml_t* xml, const char* tag, smol_xml_nodeptr_vector_t* out) {
	smol_vector_each(&xml->nodes, smol_xml_node_t, node) {
		if (strcmp(node->tag, tag) == 0) {
			smol_vector_push(out, node);
		}
	}
}

void smol_xml_find_by_attr(smol_xml_t* xml, const char* attr_name, const char* attr_value, smol_xml_nodeptr_vector_t* out) {
	smol_vector_each(&xml->nodes, smol_xml_node_t, node) {
		smol_vector_each(&node->attributes, smol_xml_attr_t, attr) {
			if (strcmp(attr->name, attr_name) == 0 && strcmp(attr->value, attr_value) == 0) {
				smol_vector_push(out, node);
				break;
			}
		}
	}
}

void smol_xml_find_by_attr_matcher(smol_xml_t* xml, const char* attr_name, smol_xml_attr_matcher_cb matcher, smol_xml_nodeptr_vector_t* out) {
	smol_vector_each(&xml->nodes, smol_xml_node_t, node) {
		smol_vector_each(&node->attributes, smol_xml_attr_t, attr) {
			if (strcmp(attr->name, attr_name) == 0 && matcher(attr->value)) {
				smol_vector_push(out, node);
				break;
			}
		}
	}
}

smol_xml_attr_t* smol_xml_get_header_attr(smol_xml_t* xml, const char* attr_name) {
	smol_vector_each(&xml->header_attributes, smol_xml_attr_t, attr) {
		if (strcmp(attr->name, attr_name) == 0) {
			return attr;
		}
	}
	return NULL;
}

void smol_xml__node_free(smol_xml_node_t* node) {
	free(node->tag);
	if (node->body) free(node->body);

	// free attribs
	smol_vector_each(&node->attributes, smol_xml_attr_t, attr) {
		free(attr->name);
		if (attr->value) free(attr->value);
	}

	smol_vector_free(&node->attributes);
	smol_vector_free(&node->children);

	memset(node, 0, sizeof(smol_xml_node_t));
}

void smol_xml_free(smol_xml_t* xml) {
	smol_vector_each(&xml->nodes, smol_xml_node_t, node) {
		smol_xml__node_free(node);
	}
	smol_vector_free(&xml->nodes);
}

#endif

#endif // SMOL_XML_H
