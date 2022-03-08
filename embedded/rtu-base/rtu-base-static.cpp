/* static version using structs */

#include <stdint.h> 
#include <string>

using namespace std;

/* global common types */

struct instancelink {
  uint16_t typeID;
  uint16_t instanceID;
};

struct Resource {
  uint16_t typeID;
  uint16_t instanceID;  
  union value {
    bool boolean_type;
    int integer_type;
    float float_type;
    string* string_type;
    instancelink link_type;
  };
  enum valueType { boolean_type, integer_type, float_type, string_type, link_type };
};

struct Object {
  uint16_t typeID;
  uint16_t instanceID;
  Resource resources[];
};
