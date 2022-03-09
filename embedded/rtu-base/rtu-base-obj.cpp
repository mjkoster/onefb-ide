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
    ValueType valueType;
    AnyValueType value;

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

    /* sync object state by copying resource values
    Source: 1. OutputValue, 2. CurrentValue, 3. InputValue
    Destination: 1. InputValue, 2. CurrentValue, 3. OutputValue
    */
    // Copy Value from input link => this Object 
    void syncInputLink() {}; 
    // Copy Value from this Object => all output links 
    void syncOutputLink() {}; 

    // Value Interface
    // Update Timer on Object
    void updateTimer(uint32_t msec) {}; 
    // Update Value 
    void updateValue(AnyValueType value) {}; // 1. InputValue, 2. CurrentValue, 3. OutputValue
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

    // Internal Interface, implements application logic
    // Handler for Timer interval
    void onInterval() {}; 
    // Handler for Value update from either input or output sync
    void onUpdate(AnyValueType value) {}; 
    // Handler to return value in response to input sync from another object
    AnyValueType onSyncInput() {
      return readValue(); // Default read value, override for e.g. gpio read
    }; 
    // Resources that are part of this object
    Resource resource[];

    // Construct with type and instance
    Object(uint16_t type, uint16_t instance) {
      typeID = type;
      instanceID = instance;
    };
};

int main() {
  Object object(9999,1);
  object.resource = {
    new Resource(1111, 1, booleanType),
    new Resource(2222, 1, integerType)
  }

};

