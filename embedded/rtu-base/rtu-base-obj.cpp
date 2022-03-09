/* rtu-base contains the base types and fundamental application types */

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

/* base classes */

/* Resources expose values */
class Resource {
  public:
    uint16_t typeID;
    uint16_t instanceID;    
    AnyValueType value;
    enum valueType { booleanType, integerType, floatType, stringType, linkType };
};

/* Objects contain a collection of resources and some bound methods */
/* Bound methods are extended for functional types */
class Object {
  public:
    uint16_t typeID;
    uint16_t instanceID;
    void init() {};
    
    // External Interface, standard implementation
    void timerUpdate() {}; // Update Timer on Object
    void inputUpdate() {}; // Fetch Value from Object
    void outputUpdate() {}; // Update Value on Object 

    // Internal Interface, implements application logic
    void onInterval() {}; // Handler for Timer interval
    void onUpdate(AnyValueType incomingValue) {}; // Handler for Value update 
    AnyValueType onInput() {
      AnyValueType returnValue;
      returnValue.integerType = 0;
      return returnValue;
    }; // Handler for Value fetch

    // Resources that are part of this object
    Resource resource[];
};

