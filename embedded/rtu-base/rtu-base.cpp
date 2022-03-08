/* rtu-base contains the base types and fundamental application types */
#include <stdint.h> 


/* global common types */

struct instancelink {
  uint16_t typeID;
  uint16_t instanceID;
};

/* base classes */

class Item {
  public:
    uint16_t typeID = 65535;
    uint16_t instanceID = 0;
    char* description;
    void initialize();
};

class Resource: public Item {
  public:
    union value {
      bool booleantype;
      int integertype;
      float floattype;
      char** stringptype;
      instancelink linktype;
    };
    enum valueType { boolean_type, integer_type, float_type, string_type, link_type };
    bool multi_instance = 0; // multi-instance capable
};
class Object: public Item {
  public:
    void syncTime() {};
    void syncOutput() {};
    void syncInput() {};
    Resource* resource_list[10];
};


/* base application type definitions */

class inputLink: public Resource {
  public:
    int typeID = 9100; // class variable?
    char* description = "Input Link pointing to an object to retrieve a value from";
    instancelink value;
};

class outputLink: public Resource {
  public:
    int typeID = 9101; // class variable?
    char* description = "Output Link pointing to an object to apply a value to";
    instancelink value;
    bool multi_instance = true;
};

class embeddedNode: public Object {
  public:
    int typeID = 9200;
    char* description = "The node object contains a list of object instances and other node-global information";
    Object* object_list[];
};

class schedulertimer: public Object {
  public:
    int typeID = 9201;
    char* description = "Timer object";
    void updateTimer(uint32_t msec) {};
    int timer_interval;
    outputLink* output_link[];
};
