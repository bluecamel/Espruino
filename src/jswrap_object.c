/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * JavaScript methods for Objects
 * ----------------------------------------------------------------------------
 */
#include "jswrap_object.h"
#include "jsparse.h"
#include "jsinteractive.h"

/*JSON{ "type":"class",
        "class" : "Object",
        "check" : "jsvIsObject(var)",
        "description" : ["This is the built-in class for Objects" ]
}*/
/*JSON{ "type":"class",
        "class" : "Function",
        "check" : "jsvIsFunction(var)",
        "description" : ["This is the built-in class for Functions" ]
}*/
/*JSON{ "type":"class",
        "class" : "Integer",
        "check" : "jsvIsInt(var)",
        "description" : ["This is the built-in class for Integer values" ]
}*/
/*JSON{ "type":"class",
        "class" : "Double",
        "check" : "jsvIsFloat(var)",
        "description" : ["This is the built-in class for Floating Point values" ]
}*/

/*JSON{ "type":"property", "class": "Object", "name" : "length",
         "description" : "Find the length of the object",
         "generate" : "jswrap_object_length",
         "return" : ["JsVar", "The value of the string"]
}*/
JsVar *jswrap_object_length(JsVar *parent) {
  if (jsvIsArray(parent)) {
    return jsvNewFromInteger(jsvGetArrayLength(parent));
  } else if (jsvIsArrayBuffer(parent)) {
      return jsvNewFromInteger(jsvGetArrayBufferLength(parent));
  } else if (jsvIsString(parent)) {
    return jsvNewFromInteger((JsVarInt)jsvGetStringLength(parent));
  }
  return 0;
}

/*JSON{ "type":"method", "class": "Object", "name" : "toString",
         "description" : "Convert the Object to a string",
         "generate" : "jswrap_object_toString",
         "return" : ["JsVar", "A String representing the object"]
}*/
JsVar *jswrap_object_toString(JsVar *parent) {
  return jsvAsString(parent, false);
}

/*JSON{ "type":"method", "class": "Object", "name" : "clone",
         "description" : "Copy this object completely",
         "generate" : "jswrap_object_clone",
         "return" : ["JsVar", "A copy of this Object"]
}*/
JsVar *jswrap_object_clone(JsVar *parent) {
  return jsvCopy(parent);
}

/*JSON{ "type":"method", "class": "Object", "name" : "on",
         "description" : ["Register an event listener for this object, for instance ```http.on('data', function(d) {...})```. See Node.js's EventEmitter."],
         "generate" : "jswrap_object_on",
         "params" : [ [ "event", "JsVar", "The name of the event, for instance 'data'"],
                      [ "listener", "JsVar", "The listener to call when this event is received"] ]
}*/
void jswrap_object_on(JsVar *parent, JsVar *event, JsVar *listener) {
  if (!jsvIsObject(parent)) {
      jsWarn("Parent must be a proper object - not a String, Integer, etc.");
      return;
    }
  if (!jsvIsString(event)) {
      jsWarn("First argument to EventEmitter.on(..) must be a string");
      return;
    }
  if (!jsvIsFunction(listener) && !jsvIsString(listener)) {
    jsWarn("Second argument to EventEmitter.on(..) must be a function or a String (containing code)");
    return;
  }
  char eventName[16] = "#on";
  jsvGetString(event, &eventName[3], sizeof(eventName)-4);

  JsVar *eventList = jsvFindChildFromString(parent, eventName, true);
  JsVar *eventListeners = jsvSkipName(eventList);
  if (jsvIsUndefined(eventListeners)) {
    // just add
    jsvSetValueOfName(eventList, listener);
  } else {
    if (jsvIsArray(eventListeners)) {
      // we already have an array, just add to it
      jsvArrayPush(eventListeners, listener);
    } else {
      // not an array - we need to make it an array
      JsVar *arr = jsvNewWithFlags(JSV_ARRAY);
      jsvArrayPush(arr, eventListeners);
      jsvArrayPush(arr, listener);
      jsvSetValueOfName(eventList, arr);
      jsvUnLock(arr);
    }
  }
  jsvUnLock(eventListeners);
  jsvUnLock(eventList);
}

/*JSON{ "type":"method", "class": "Object", "name" : "emit",
         "description" : ["Call the event listeners for this object, for instance ```http.emit('data', 'Foo')```. See Node.js's EventEmitter."],
         "generate" : "jswrap_object_emit",
         "params" : [ [ "event", "JsVar", "The name of the event, for instance 'data'"],
                      [ "v1", "JsVar", "Optional argument 1"],
                      [ "v2", "JsVar", "Optional argument 2"]  ]
}*/
void jswrap_object_emit(JsVar *parent, JsVar *event, JsVar *v1, JsVar *v2) {
  if (!jsvIsObject(parent)) {
      jsWarn("Parent must be a proper object - not a String, Integer, etc.");
      return;
    }
  if (!jsvIsString(event)) {
    jsWarn("First argument to EventEmitter.emit(..) must be a string");
    return;
  }
  char eventName[16] = "#on";
  jsvGetString(event, &eventName[3], sizeof(eventName)-4);
  jsiQueueObjectCallbacks(parent, eventName, v1, v2);
}

/*JSON{ "type":"method", "class": "Object", "name" : "removeAllListeners",
         "description" : ["Removes all listeners, or those of the specified event."],
         "generate" : "jswrap_object_removeAllListeners",
         "params" : [ [ "event", "JsVar", "The name of the event, for instance 'data'"] ]
}*/
void jswrap_object_removeAllListeners(JsVar *parent, JsVar *event) {
  if (!jsvIsObject(parent)) {
      jsWarn("Parent must be a proper object - not a String, Integer, etc.");
      return;
    }
  if (jsvIsString(event)) {
    // remove the whole child containing listeners
    char eventName[16] = "#on";
    jsvGetString(event, &eventName[3], sizeof(eventName)-4);
    JsVar *eventList = jsvFindChildFromString(parent, eventName, true);
    if (eventList) {
      jsvRemoveChild(parent, eventList);
      jsvUnLock(eventList);
    }
  } else if (jsvIsUndefined(event)) {
    // Eep. We must remove everything beginning with '#on'
    JsObjectIterator it;
    jsvObjectIteratorNew(&it, parent);
    while (jsvObjectIteratorHasElement(&it)) {
      JsVar *key = jsvObjectIteratorGetKey(&it);
      jsvObjectIteratorNext(&it);
      if (jsvIsString(key) &&
          key->varData.str[0]=='#' &&
          key->varData.str[1]=='o' &&
          key->varData.str[2]=='n') {
        // begins with #on - we must kill it
        jsvRemoveChild(parent, key);
      }
      jsvUnLock(key);
    }
    jsvObjectIteratorFree(&it);
  } else {
    jsWarn("First argument to EventEmitter.removeAllListeners(..) must be a string, or undefined");
    return;
  }
}

/*JSON{ "type":"method", "class": "Function", "name" : "replaceWith",
         "description" : ["This replaces the function with the one in the argument - while keeping the old function's scope. This allows inner functions to be edited, and is used when edit() is called on an inner function."],
         "generate" : "jswrap_function_replaceWith",
         "params" : [ [ "newFunc", "JsVar", "The new function to replace this function with"] ]
}*/
void jswrap_function_replaceWith(JsVar *oldFunc, JsVar *newFunc) {
  if (!jsvIsFunction(newFunc)) {
    jsWarn("First argument of replaceWith should be a function - ignoring");
    return;
  }
  // Grab scope - the one thing we want to keep
  JsVar *scope = jsvFindChildFromString(oldFunc, JSPARSE_FUNCTION_SCOPE_NAME, false);
  // so now remove all existing entries
  jsvRemoveAllChildren(oldFunc);
  // now re-add scope
  jsvAddName(oldFunc, scope);
  jsvUnLock(scope);
  // now re-add other entries
  JsObjectIterator it;
  jsvObjectIteratorNew(&it, newFunc);
  while (jsvObjectIteratorHasElement(&it)) {
    JsVar *el = jsvObjectIteratorGetKey(&it);
    jsvObjectIteratorNext(&it);
    if (!jsvIsStringEqual(el, JSPARSE_FUNCTION_SCOPE_NAME)) {
      JsVar *copy = jsvCopy(el);
      if (copy) {
        jsvAddName(oldFunc, copy);
        jsvUnLock(copy);
      }
    }
  }
  jsvObjectIteratorFree(&it);

}
