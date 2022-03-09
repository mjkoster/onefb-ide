/* static version using structs */

#include <stdint.h> 

/* global common types */

struct InstanceLink {
  uint16_t typeID;
  uint16_t instanceID;
};

union AnyValueType {
  bool booleanType;
  int integerType;
  float floatType;
  char* stringType;
  InstanceLink linkType;
};

struct Resource {
  uint16_t typeID;
  uint16_t instanceID;  
  AnyValueType value;
  enum valueType { booleanType, integerType, floatType, stringType, linkType };
};

struct Object {
  uint16_t typeID;
  uint16_t instanceID;
  Resource resource[];
};
