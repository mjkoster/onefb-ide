/* rtu-base contains the base types and fundamental application types */
#include <stdint.h> 
#include <string>
/* global common types */

using namespace std;

struct instancelink {
  uint16_t typeID;
  uint16_t instanceID;
};

/* base classes */

class Item {
  public:
    uint16_t typeID;
    uint16_t instanceID;
    string description;
    void initialize() {};
};

class Resource: public Item {
  public:
    union value {
      bool boolean_type;
      int integer_type;
      float float_type;
      string* string_type;
      instancelink link_type;
    };
    enum valueType { boolean_type, integer_type, float_type, string_type, link_type };
};

class Object: public Item {
  public:
    void syncTime() {};
    void syncOutput() {};
    void syncInput() {};
    Resource* resource_list[];
};

