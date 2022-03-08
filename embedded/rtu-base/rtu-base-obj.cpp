/* rtu-base contains the base types and fundamental application types */
#include <stdint.h> 
#include <string>
/* global common types */

using namespace std;

struct instancelink {
  uint16_t typeID;
  uint16_t instanceID;
};

union value {
  bool boolean_type;
  int integer_type;
  float float_type;
  string* string_type;
  instancelink link_type;
};

/* base classes */

class Item {
  public:
    uint16_t typeID;
    uint16_t instanceID;
    string description;
    void initialize() {};
};

/* Resources expose values */
class Resource: public Item {
  public:
    value resourceValue;
    enum valueType { boolean_type, integer_type, float_type, string_type, link_type };
};

/* Objects contain a collection of resources and some bound methods */
/* Bound methods are extended for functional types */
class Object: public Item {
  public:
    // External Interface
    void timerUpdate() {}; // Update Timer on Object
    void inputUpdate() {}; // Fetch Value from Object
    void outputUpdate() {}; // Update Value on Object
    // Internal Interface
    void onInterval() {}; // Handler for Timer interval
    void onUpdate(value incomingValue) {}; // Handler for Value update 
    value onInput() {}; // Handler for Value fetch
    Resource* resourceList[];
};

