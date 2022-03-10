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

/* Well-known Resource Types, make a header from the SDF translator */
  const uint16_t InputLinkType = 13000;
  const uint16_t OutputLinkType = 13001;
  const uint16_t InputValueType = 13002;
  const uint16_t OutputValueType = 13003;
  const uint16_t CurrentValueType = 13004;
  const uint16_t CurrentTimeType = 13005;
  const uint16_t IntervalTimeType = 13006;
  const uint16_t LastActivationTimeType = 13007;

/* base classes */

/* Resources expose values and chain together into a linked list */
class Resource {
  public:
    uint16_t typeID;
    uint16_t instanceID;    
    Resource* nextResource;

    ValueType valueType;
    AnyValueType value;

    // Construct with type and instance + value type
    Resource(uint16_t type, uint16_t instance, ValueType vtype) {
      typeID = type;
      instanceID = instance;
      valueType = vtype;
      nextResource = NULL;
    };
};

/* Objects contain a collection of resources and some bound methods and chain into a linked list */
/* Bound methods are extended for application function types */
class Object {
  public:
    uint16_t typeID; // could be private 
    uint16_t instanceID;
    Object* nextObject;

    /* sync object state by copying Default resource values
    Source: 1. OutputValue, 2. CurrentValue, 3. InputValue
    Destination: 1. InputValue, 2. CurrentValue, 3. OutputValue
    */

    // Copy Value from input link => this Object 
    void syncFromInputLink() {
      // readDefaultValue from InputLink
      // writeDefaultValue to this object
    }; 

    // Copy Value from this Object => all output links 
    void syncToOutputLink() {
      // readDefaultValue from this object
      // updateDefaultValue to OutputLink(s)
    }; 

    // Value Interface

    // Update Timer on Object
    void updateTimer(uint32_t msec) {}; 
      // update current time
      // interval time check is wrap-safe if time variable and HW timer both have uint32_t wrap behavior
      // if(current time >= last activation time + interval time){ update last activation time and call onInterval }
    
    // Update Value 
    void updateDefaultValue(AnyValueType value) {
      // 1. InputValue, 2. CurrentValue, 3. OutputValue
      // if (upadteValueByID (InputValueType,0,value) != NULL) {return}
      // etc.
    }; 
    void updateValueByID(uint16_t type, uint16_t instance, AnyValueType value) {
      Resource* resource = getResourceByID(type, instance);
      if (resource != NULL) {
        resource -> value = value;
      }
      else {
        printf("NULL in updateValueByID\n");
      };
    };

    // Read Value
    AnyValueType readDefaultValue() {
      AnyValueType returnValue;
      // if (returnValue = readValueByID(OutputValueType,0) != NULL)
      // etc.
      returnValue.integerType = 0; // 1. OutputValue, 2. CurrentValue, 3. InputValue
      return returnValue;
    }; 
    AnyValueType readValueByID(uint16_t type, uint16_t instance) {
      Resource* resource = getResourceByID(type, instance);
      if (resource != NULL) {
        return resource -> value;
      }
      else {
        printf ("NULL in readValueByID\n");
        return resource -> value;
      }
    }; 

    // Internal Interface, implements application logic
    
    // Handler for Timer Interval
    void onInterval() {}; 

    // Handler for Value update from either input or output sync
    void onValueUpdate(AnyValueType value) {}; 

    // Handler to return value in response to input sync from another object
    AnyValueType onInputSync() {
      return readDefaultValue(); // Default read value, override for e.g. gpio read
    }; 

    // Linked list of Resources that are part of this object, make private?
    Resource* nextResource;

    Resource* newResource(uint16_t type, uint16_t instance, ValueType vtype) {
      // find last resource in the chain
      if (nextResource == NULL) {
        nextResource = new Resource(type, instance, vtype );
        return nextResource;
      }
      else {
        Resource* resource = nextResource;
          while (resource -> nextResource != NULL) {
            resource = resource -> nextResource;
          };
        // make instance and add the new resource
        resource -> nextResource = new Resource(type, instance, vtype );
        return resource -> nextResource;
      };
    };

    Resource* getResourceByID(uint16_t type, uint16_t instance) {
      Resource* resource = nextResource;
      while ( (resource != NULL) && (resource -> typeID != type || resource -> instanceID != instance) ) {
        resource = resource -> nextResource;
      };
      return resource; // returns NULL if resource doesn't exist
    };

    // Construct with type and instance and empty list
    Object(uint16_t type, uint16_t instance) {
      typeID = type;
      instanceID = instance;
      nextResource = NULL;
      nextObject = NULL;
    };
};

class ObjectList {
  public:
    // Linked list of Objects that are part of this RTU
    Object* nextObject; // private?
    // make a new object and add it to the list
    Object* newObject(uint16_t type, uint16_t instance) {
      // find the last object in the chain, has a null nextobject pointer
      // FIXME check if it already exists?
      if (nextObject == NULL) {
        nextObject = new Object(type, instance);
        return nextObject;
      }
      else {
        Object* object = nextObject;
        while (object -> nextObject != NULL) {
          object = object -> nextObject;
        };
        // make instance and add the new resource
        object -> nextObject = new Object(type, instance );
        return object -> nextObject; 
      };     
    };

    // return a pointer to the first object that matches the type and instance
    Object* getObjectByID(uint16_t type, uint16_t instance) {
      Object* object = nextObject;
      while (object != NULL && object -> typeID != type && object -> instanceID != instance) {
        object = object -> nextObject;
      };
      return object; // returns NULL if doesn't exist
    };

    void displayObjects() {
      Object* object = nextObject;
      while ( object != NULL) {
        printf ( "[%d, %d]\n", object -> typeID, object -> instanceID);
        Resource* resource = object -> nextResource;
        while ( resource != NULL) {
          printf ( "  [%d, %d] ", resource -> typeID, resource -> instanceID);
          printf ( "value type %d, ", resource -> valueType);
          printf ( "value = ");
          switch(resource -> valueType) {
            case booleanType: {
              printf ( "%s\n", resource -> value.booleanType ? "true": "false");
              break;
            }
            case integerType: {
              printf ( "%d\n", resource -> value.integerType);
              break;
            }
            case floatType: {
              printf ( "%f\n", resource -> value.floatType);
              break;
            }
            default:
              printf ("\n");
          }
          resource = resource -> nextResource;
        };
        object = object -> nextObject;
      };
    };

    // construct with an empty object list
    ObjectList() {
      nextObject = NULL;
    };
};

int main() {
  ObjectList rtu;
  AnyValueType value;
  Object* object = rtu.newObject(9999,1);
  object -> newResource(1111, 1, integerType);
  object -> newResource(2222, 2, floatType);
  value.integerType = 100;
  object -> updateValueByID(1111, 1, value);
  value.floatType = 101.1;
  object -> updateValueByID(2222, 2, value);
  object = rtu.newObject(9090,2);
  object -> newResource(1010, 1, integerType);
  object -> newResource(1010, 2, floatType);
  value.integerType = 1001;
  object -> updateValueByID(1010, 1, value);
  value.floatType = 1001.01;
  object -> updateValueByID(1010, 2, value);

  rtu.displayObjects();
  return(0);
};

