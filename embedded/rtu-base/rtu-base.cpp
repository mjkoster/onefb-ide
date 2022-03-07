/* rtu-base contains the base types and fundamental application types */


/* global common types */

struct instancelink {
  uint16_t typeID,
  uint16_t instanceID
}

/* base classes */

class onefbItem {
  public:
    uint16_t typeID = 65535 ;
    uint16_t instanceID = 0;
    string description;
    void initialize();
}

class Resource: public onefbItem {
  public:
    union value {
      bool booleantype,
      int integertype,
      float floattype,
      string* stringptype,
      instancelink linktype
    enum valueType { boolean, integer, float, string, link };
    bool multi_instance = false; // multi-instance capable
    };

};
class Object: public onefbItem {
  public:
    Resource* resource_list[];
    void syncTime() {};
    void syncOutput() {};
    void syncInput() {};
};


/* base application type definitions */

class inputLink: public Resource {
  public:
    typeID = 9100; // class variable?
    description = "Input Link pointing to an object to retrieve a value from"
    instancelink value;
};

class outputLink: public Resource {
  public:
    typeID = 9101; // class variable?
    description = "Output Link pointing to an object to apply a value to"
    instancelink value;
    multi_instance = true;
};

class embeddedNode: public Object {
  public:
    typeID = 9200;
    description = "The node object contains a list of object instances and other node-global information"
    Object* object_list[];
};

class schedulertimer: public Object {
  public:
    typeID = 9201;
    description = "Timer object";
    void updateTimer(time_t msec) {};
    int timer_interval;
    outputLink* output_link[];
};
