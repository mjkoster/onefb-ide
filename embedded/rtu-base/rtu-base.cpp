/* rtu-base contains the base types and fundamental application types */
#include <stdint.h> 
#include <string>
#include <vector>
/* global common types */

using namespace std;

struct instancelink {
  uint16_t typeID;
  uint16_t instanceID;
};

/* base classes */

class Item {
  public:
    uint16_t typeID = 65535;
    uint16_t instanceID = 0;
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
    bool multi_instance = 0; // multi-instance capable
};
class Object: public Item {
  public:
    void syncTime() {};
    void syncOutput() {};
    void syncInput() {};
    vector<Resource*> resources;
    Resource* resource_list[0];
};

/* base application type definitions */

class inputLink: public Resource {
  public:
    uint16_t typeID = 9100; // class variable?
    string description = "Input Link pointing to an object to retrieve a value from";
    instancelink value;
};

class outputLink: public Resource {
  public:
    uint16_t typeID = 9101; // class variable?
    string description = "Output Link pointing to an object to apply a value to";
    instancelink value;
    bool multi_instance = true;
};

class embeddedNode: public Object {
  public:
    uint16_t typeID = 9200;
    string description = "The node object contains a list of object instances and other node-global information";
    vector<Object*> objects;
    Object* object_list[0];
};

class schedulertimer: public Object {
  public:
    uint16_t typeID = 9201;
    string description = "Timer object";
    void updateTimer(uint32_t msec) {};
    uint32_t timer_interval;
    vector<outputLink*> output_links;
    outputLink* output_link[0];
};
