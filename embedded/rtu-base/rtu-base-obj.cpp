/* rtu-base contains the base types and fundamental application types */

#include <stdint.h> 
#include <stdio.h> 

/* global common types */

struct InstanceLink {
  uint16_t typeID;
  uint16_t instanceID;
};

enum ValueType { booleanType, integerType, floatType, stringType, linkType, timeType };

union AnyValueType {
  bool booleanType;
  int integerType;
  double floatType;
  char* stringType;
  InstanceLink linkType;
  uint32_t timeType;
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
    void syncFromInputLink() {}; 
    // Copy Value from this Object => all output links 
    void syncToOutputLink() {}; 

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
    // Handler for Timer update
    void onTimerUpdatel() {}; 
    // Handler for Value update from either input or output sync
    void onUpdate(AnyValueType value) {}; 
    // Handler to return value in response to input sync from another object
    AnyValueType syncInput() {
      return readValue(); // Default read value, override for e.g. gpio read
    }; 
    // Resources that are part of this object
    Resource* resource[];

    // Construct with type and instance
    Object(uint16_t type, uint16_t instance) {
      typeID = type;
      instanceID = instance;
    };
};

int main() {
  Object object(9999,1); // need a place to put a list of objects
  object.resource[0] = new Resource(1111, 1, integerType);
  object.resource[1] = new Resource(2222, 1, floatType);
  Object* object2 = new Object(9999,2);
  object2 -> resource[0] = new Resource(1111, 1, integerType); 
  object2 -> resource[1] = new Resource(2222, 1, floatType);

  object.resource[0] -> value.integerType = 100;
  object.resource[1] -> value.floatType = 101.1;
  object2 -> resource[0] -> value.integerType = 200;
  object2 -> resource[1] -> value.floatType = 201.1;
  printf("r0type = %d\n", object.resource[0] -> typeID);
  printf("r1type = %d\n", object.resource[1] -> typeID);
  printf("r0value = %d\n", object.resource[0] -> value.integerType);
  printf("r1value = %f\n", object.resource[1] -> value.floatType);
  printf("r0type2 = %d\n", object2 -> resource[0] -> typeID);
  printf("r1type2 = %d\n", object2 -> resource[1] -> typeID);
  printf("r0value2 = %d\n", object2 -> resource[0] -> value.integerType);
  printf("r1value2 = %f\n", object2 -> resource[1] -> value.floatType);
  return(0);
};

