/* rtu-base contains the base types and fundamental application types */

#include <stdint.h> 

/* global common types */

struct InstanceLink {
  uint16_t typeID;
  uint16_t instanceID;
};

enum ValueType { booleanType, integerType, floatType, stringType, linkType };

union AnyValueType {
  bool booleanType;
  int integerType;
  float floatType;
  char* stringType;
  InstanceLink linkType;
};

/* base classes */

/* Resources expose values */
class Resource {
  public:
    uint16_t typeID;
    uint16_t instanceID;    
    AnyValueType value;
    ValueType valueType;

    // Construct with type and instance + value type
    Resource(uint16_t type, uint16_t instance, ValueType vtype) {
      typeID = type;
      instanceID = instance;
      valueType = vtype;
    };
};

/* Objects contain a collection of resources and some bound methods */
/* Bound methods are extended for application functional types */
class Object {
  public:
    uint16_t typeID;
    uint16_t instanceID;
    
    // External Interface, standard implementation
    // Update Timer on Object
    void updateTimer(uint32_t msec) {}; 
    // Update (Input, Current, Output) Value 
    void updateValue(AnyValueType value) {};
    void updateValueByID(uint16_t type, uint16_t instance, AnyValueType value) {};
    // Read Value
    AnyValueType readValue() {
      AnyValueType returnValue;
      returnValue.integerType = 0; // 1. OutputValue, 2. CurrentValue, 3. InputValue
      return returnValue;
    }; 
    AnyValueType readValueByID(uint16_t type, uint16_t instance) {
      AnyValueType returnValue;
      returnValue.integerType = 0;
      return returnValue;
    }; 

    // Copy (Output, Current, Input) => (Input, Current, Output) Value from input link
    void syncInputLink() {}; 
    // Copy (Output, Current, Input) => (Input, Current, Output) Value to all output links 
    void syncOutputLink() {}; 

    // Internal Interface, implements application logic
    // Handler for Timer interval
    void onInterval() {}; 
    // Handler for Value update from either input or output sync
    void onUpdate(AnyValueType value) {}; 
    // Handler to return input link
    AnyValueType onSyncInput() {
      AnyValueType returnValue;
      returnValue.integerType = 0; // 1. OutputValue, 2. CurrentValue, 3. InputValue
      return returnValue;
    }; 

    // Construct with type and instance
    Object(uint16_t type, uint16_t instance) {
      typeID = type;
      instanceID = instance;
    };

    // Resources that are part of this object
    Resource resource[];
};

