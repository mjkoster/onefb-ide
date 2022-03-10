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

/* Resource: expose values and chain together into a linked list for each object*/
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
    Object* nextObject; // next Object in the chain
    Object* firstObject; // first Object in the ObjectList
   
    // return a pointer to the first object that matches the type and instance
    Object* getObjectByID(uint16_t type, uint16_t instance) {
      Object* object = firstObject;
      while (object != NULL && (object -> typeID != type || object -> instanceID != instance)) {
        object = object -> nextObject;
      };
      return object; // returns NULL if doesn't exist
    };

    // Copy Value from input link => this Object 
    void syncFromInputLink() {
      // readDefaultValue from InputLink
      // updateDefaultValue on this object
      Resource* inputLink = getResourceByID(InputLinkType,0);
      if (inputLink != NULL) {
        Object* sourceObject = getObjectByID(inputLink -> value.linkType.typeID, inputLink -> value.linkType.instanceID);
        updateDefaultValue(sourceObject -> onInputSync());
      }
    }; 

    // Copy Value from this Object => all output links 
    void syncToOutputLink() {
      // readDefaultValue from this object
      // updateDefaultValue to OutputLink(s)
     AnyValueType value = readDefaultValue();
      Resource* resource = nextResource;
        while ( (resource != NULL) ) {
          if (OutputLinkType == resource -> typeID) { // process all output links
            Object* object = getObjectByID(resource -> value.linkType.typeID, resource -> value.linkType.instanceID);
            object -> updateDefaultValue(value);
          };
        resource = resource -> nextResource;
      };
 
      if (resource != NULL) {
        //Object* object = 
        //resource -> value.linkType.typeID;
      }    
    }; 

    // Value Interface

    // Update Timer on Object
    void updateTimer(uint32_t msec) {}; 
      // update current time
      // interval time check is wrap-safe if time variable and HW timer both have uint32_t wrap behavior
      // if(current time >= last activation time + interval time){ update last activation time and call onInterval }
    
    // Update Value 
    void updateDefaultValue(AnyValueType value) {
      // priority of resource type to update value and call onUpdate
      Resource* resource = getResourceByID(InputValueType,0);
      if (resource != NULL) {
        resource -> value = value;
        onValueUpdate(resource -> typeID, resource -> instanceID, value);
        return;
      };
      resource = getResourceByID(CurrentValueType,0);
      if (resource != NULL) {
        resource -> value = value;
        onValueUpdate(resource -> typeID, resource -> instanceID, value);
        return;
      };
      resource = getResourceByID(OutputValueType,0);
      if (resource != NULL){
        resource -> value = value;
        onValueUpdate(resource -> typeID, resource -> instanceID, value);
        return;
      };
      printf("updateDefault couldn't find a candidate resource\n");
      return;
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
      Resource* resource = getResourceByID(OutputValueType,0);
      if (resource != NULL) {
        return(resource -> value);
      };
      resource = getResourceByID(CurrentValueType,0);
      if (resource != NULL) {
        return(resource -> value);
      };
      resource = getResourceByID(InputValueType,0);
      if (resource != NULL){
        return(resource -> value);
      };
      printf("readDefault couldn't find a candidate resource\n");
      return(returnValue);
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
    void onValueUpdate(uint16_t type, uint16_t instance, AnyValueType value) {}; 

    // Handler to return value in response to input sync from another object
    AnyValueType onInputSync() {
      return readDefaultValue(); // Default read value, override for e.g. gpio read
    }; 

    // Linked list of Resources that are part of this object, make private?
    Resource* nextResource;

    Resource* newResource(uint16_t type, uint16_t instance, ValueType vtype) {
      // find last resource in the chain
      if (nextResource == NULL) { // make first resource instance and add to the list
        nextResource = new Resource(type, instance, vtype );
        return nextResource;
      }
      else { // already have first resource
        Resource* resource = nextResource;
          while (resource -> nextResource != NULL) {
            resource = resource -> nextResource;
          };
        // make instance and add the new resource to the list
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
    Object(uint16_t type, uint16_t instance, Object* listNextObject) {
      typeID = type;
      instanceID = instance;
      nextResource = NULL;
      nextObject = NULL;
      firstObject = listNextObject;
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
      if (nextObject == NULL) { // make first object and add to the list
        nextObject = new Object(type, instance, nextObject);
        return nextObject;
      }
      else { // already have the first object, find the end of the list 
        Object* object = nextObject;
        while (object -> nextObject != NULL) {
          object = object -> nextObject;
        };
        // make instance and add the new resource
        object -> nextObject = new Object(type, instance, nextObject);
        return object -> nextObject; 
      };     
    };

    // return a pointer to the first object that matches the type and instance
    Object* getObjectByID(uint16_t type, uint16_t instance) {
      Object* object = nextObject;
      while (object != NULL && (object -> typeID != type || object -> instanceID != instance)) {
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
  Object* object = rtu.newObject(9999,1);
  object -> newResource(1111, 1, integerType);
  object -> updateValueByID(1111, 1, (AnyValueType){.integerType=100});
  object -> newResource(2222, 2, floatType);
  object -> updateValueByID(2222, 2, (AnyValueType){.floatType=101.1});
  object = rtu.newObject(9090,2);
  object -> newResource(1010, 1, integerType);
  object -> updateValueByID(1010, 1, (AnyValueType){.integerType=1001});
  object -> newResource(1010, 2, floatType);
  object -> updateValueByID(1010, 2, (AnyValueType){.floatType=1001.01});

  rtu.displayObjects();
  return(0);
};

