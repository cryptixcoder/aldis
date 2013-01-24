/**
 * @file
 * This file defines the class ProxyBusObject.
 * The ProxyBusObject represents a single object registered  registered on the bus.
 * ProxyBusObjects are used to make method calls on these remotely located DBus objects.
 */

/******************************************************************************
 * Copyright 2009-2011, Qualcomm Innovation Center, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 ******************************************************************************/
#ifndef _ALLJOYN_C_REMOTEBUSOBJECT_H
#define _ALLJOYN_C_REMOTEBUSOBJECT_H

#include <qcc/platform.h>
#include <alljoyn_c/AjAPI.h>
#include <alljoyn_c/InterfaceDescription.h>
#include <alljoyn_c/MessageReceiver.h>
#include <alljoyn_c/MsgArg.h>
#include <alljoyn_c/Session.h>

#include <Status.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _alljoyn_proxybusobject_handle*              alljoyn_proxybusobject;
#ifndef _ALLJOYN_OPAQUE_BUSATTACHMENT_
#define _ALLJOYN_OPAQUE_BUSATTACHMENT_
typedef struct _alljoyn_busattachment_handle*               alljoyn_busattachment;
#endif

/**
 * Callback registered with alljoyn_proxybusobject_introspectremoteobjectasync()
 *
 * @param status    ER_OK if successful
 * @param obj       Remote bus object that was introspected
 * @param context   Context passed in IntrospectRemoteObjectAsync()
 */
typedef void (*alljoyn_proxybusobject_listener_introspectcb_ptr)(QStatus status, alljoyn_proxybusobject obj, void* context);

/**
 * Create an empty proxy bus object that refers to an object at given remote service name. Note
 * that the created proxy bus object does not contain information about the interfaces that the
 * actual remote object implements with the exception that org.freedesktop.DBus.Peer
 * interface is special-cased (per the DBus spec) and can always be called on any object. Nor
 * does it contain information about the child objects that the actual remote object might
 * contain.
 *
 * To fill in this object with the interfaces and child object names that the actual remote
 * object describes in its introspection data, call IntrospectRemoteObject() or
 * IntrospectRemoteObjectAsync().
 *
 * @param bus        The bus.
 * @param service    The remote service name (well-known or unique).
 * @param path       The absolute (non-relative) object path for the remote object.
 * @param sessionId  The session id the be used for communicating with remote object.
 */
extern AJ_API alljoyn_proxybusobject alljoyn_proxybusobject_create(alljoyn_busattachment bus, const char* service,
                                                                   const char* path, alljoyn_sessionid sessionId);

/**
 * Destroy a proxy object created using alljoyn_proxybusobject_create.
 *
 * @param proxyObj The proxy bus object to destroy.
 */
extern AJ_API void alljoyn_proxybusobject_destroy(alljoyn_proxybusobject proxyObj);

/**
 * Add an interface to this ProxyBusObject.
 *
 * Occasionally, AllJoyn library user may wish to call a method on
 * a %ProxyBusObject that was not reported during introspection of the remote object.
 * When this happens, the InterfaceDescription will have to be registered with the
 * Bus manually and the interface will have to be added to the %ProxyBusObject using this method.
 * @remark
 * The interface added via this call must have been previously registered with the
 * Bus. (i.e. it must have come from a call to alljoyn_busattachment_getinterface).
 *
 * @param proxyObj      The proxy bus object onto which the interface is to be added.
 * @param iface    The interface to add to this object. Must come from alljoyn_busattachment_getinterface.
 * @return
 *      - #ER_OK if successful.
 *      - An error status otherwise
 */
extern AJ_API QStatus alljoyn_proxybusobject_addinterface(alljoyn_proxybusobject proxyObj, const alljoyn_interfacedescription iface);

/**
 * Add an existing interface to this object using the interface's name.
 *
 * @param proxyObj      The proxy bus object onto which the interface is to be added.
 * @param name   Name of existing interface to add to this object.
 * @return
 *      - #ER_OK if successful.
 *      - An error status otherwise.
 */
extern AJ_API QStatus alljoyn_proxybusobject_addinterface_by_name(alljoyn_proxybusobject proxyObj, const char* name);

/**
 * Get a path descendant ProxyBusObject (child) by its relative path name.
 *
 * For example, if this ProxyBusObject's path is @c "/foo/bar", then you can
 * retrieve the ProxyBusObject for @c "/foo/bar/bat/baz" by calling
 * @c GetChild("bat/baz")
 *
 * @param proxyObj The proxybus object we are getting the descendant from
 * @param path     The relative path for the child.
 *
 * @return
 *      - The (potentially deep) descendant ProxyBusObject
 *      - NULL if not found.
 */
extern AJ_API alljoyn_proxybusobject alljoyn_proxybusobject_getchild(alljoyn_proxybusobject proxyObj, const char* path);

/**
 * Add a child object (direct or deep object path descendant) to this object.
 * If you add a deep path descendant, this method will create intermediate
 * ProxyBusObject children as needed.
 *
 * @remark
 *  - It is an error to try to add a child that already exists.
 *  - It is an error to try to add a child that has an object path that is not a descendant of this object's path.
 *
 * @param proxyObj The proxy bus object onto which the child object is to be added
 * @param child    Child ProxyBusObject
 *
 * @return
 *      - #ER_OK if successful.
 *      - #ER_BUS_BAD_CHILD_PATH if the path is a bad path
 *      - #ER_BUS_OBJ_ALREADY_EXISTS the the object already exists on the ProxyBusObject
 */
extern AJ_API QStatus alljoyn_proxybusobject_addchild(alljoyn_proxybusobject proxyObj, const alljoyn_proxybusobject child);

/**
 * Remove a child object and any descendants it may have.
 *
 * @param proxyObj The proxy bus object off of which the child object is to be removed
 * @param path     Absolute or relative (to this ProxyBusObject) object path.
 *
 * @return
 *      - #ER_OK if successful.
 *      - #ER_BUS_BAD_CHILD_PATH if the path given was not a valid path
 *      - #ER_BUS_OBJ_NOT_FOUND if the Child object was not found
 *      - #ER_FAIL any other unexpected error.
 */
extern AJ_API QStatus alljoyn_proxybusobject_removechild(alljoyn_proxybusobject proxyObj, const char* path);

/**
 * Query the remote object on the bus to determine the interfaces and
 * children that exist. Use this information to populate this proxy's
 * interfaces and children.
 *
 * This call causes messages to be send on the bus, therefore it cannot
 * be called within AllJoyn callbacks (method/signal/reply handlers or
 * ObjectRegistered callbacks, etc.)
 *
 * @param proxyObj    The proxy bus object that will query the remote object.
 *
 * @return
 *      - #ER_OK if successful
 *      - An error status otherwise
 */
extern AJ_API QStatus alljoyn_proxybusobject_introspectremoteobject(alljoyn_proxybusobject proxyObj);

/**
 * Query the remote object on the bus to determine the interfaces and
 * children that exist. Use this information to populate this object's
 * interfaces and children.
 *
 * This call executes asynchronously. When the introspection response
 * is received from the actual remote object, this ProxyBusObject will
 * be updated and the callback will be called.
 *
 * This call exists primarily to allow introspection of remote objects
 * to be done inside AllJoyn method/signal/reply handlers and ObjectRegistered
 * callbacks.
 *
 * @param proxyObj    The proxy bus object that will query the remote object.
 * @param callback  Method on listener that will be called.
 * @param context   User defined context which will be passed as-is to callback.
 * @return
 *      - #ER_OK if successful.
 *      - An error status otherwise
 */
extern AJ_API QStatus alljoyn_proxybusobject_introspectremoteobjectasync(alljoyn_proxybusobject proxyObj, alljoyn_proxybusobject_listener_introspectcb_ptr callback, void* context);
/**
 * Get a property from an interface on the remote object.
 *
 * @param proxyObj    The proxy bus object the property will be read from
 * @param iface       Name of interface to retrieve property from.
 * @param property    The name of the property to get.
 * @param[out] value  Property value.
 *
 * @return
 *      - #ER_OK if the property was obtained.
 *      - #ER_BUS_OBJECT_NO_SUCH_INTERFACE if the no such interface on this remote object.
 *      - #ER_BUS_NO_SUCH_PROPERTY if the property does not exist
 */
extern AJ_API QStatus alljoyn_proxybusobject_getproperty(alljoyn_proxybusobject proxyObj, const char* iface, const char* property, alljoyn_msgarg value);

/**
 * Get all properties from an interface on the remote object.
 *
 * @param proxyObj    The proxy bus object the properties will be read from
 * @param iface       Name of interface to retrieve all properties from.
 * @param[out] values Property values returned as an array of dictionary entries, signature "a{sv}".
 *
 * @return
 *      - #ER_OK if the property was obtained.
 *      - #ER_BUS_OBJECT_NO_SUCH_INTERFACE if the no such interface on this remote object.
 *      - #ER_BUS_NO_SUCH_PROPERTY if the property does not exist
 */
extern AJ_API QStatus alljoyn_proxybusobject_getallproperties(alljoyn_proxybusobject proxyObj, const char* iface, alljoyn_msgarg values);

/**
 * Set a property on an interface on the remote object.
 *
 * @param proxyObj  The proxy bus object the property will be set on
 * @param iface     Interface that holds the property
 * @param property  The name of the property to set
 * @param value     The value to set
 *
 * @return
 *      - #ER_OK if the property was set
 *      - #ER_BUS_OBJECT_NO_SUCH_INTERFACE if the no such interface on this remote object.
 *      - #ER_BUS_NO_SUCH_PROPERTY if the property does not exist
 */
extern AJ_API QStatus alljoyn_proxybusobject_setproperty(alljoyn_proxybusobject proxyObj, const char* iface, const char* property, alljoyn_msgarg value);

/**
 * Make a synchronous method call
 *
 * @param proxyObj     ProxyBusObject on which to call the method.
 * @param ifaceName    Name of interface.
 * @param methodName   Name of method.
 * @param args         The arguments for the method call (can be NULL)
 * @param numArgs      The number of arguments
 * @param replyMsg     The reply message received for the method call
 * @param timeout      Timeout specified in milliseconds to wait for a reply
 * @param flags        Logical OR of the message flags for this method call. The following flags apply to method calls:
 *                     - If #ALLJOYN_FLAG_ENCRYPTED is set the message is authenticated and the payload if any is encrypted.
 *                     - If #ALLJOYN_FLAG_COMPRESSED is set the header is compressed for destinations that can handle header compression.
 *                     - If #ALLJOYN_FLAG_AUTO_START is set the bus will attempt to start a service if it is not running.
 *
 * @return
 *      - #ER_OK if the method call succeeded and the reply message type is #MESSAGE_METHOD_RET
 *      - #ER_BUS_REPLY_IS_ERROR_MESSAGE if the reply message type is #MESSAGE_ERROR
 */
extern AJ_API QStatus alljoyn_proxybusobject_methodcall(alljoyn_proxybusobject proxyObj,
                                                        const char* ifaceName,
                                                        const char* methodName,
                                                        const alljoyn_msgarg args,
                                                        size_t numArgs,
                                                        alljoyn_message replyMsg,
                                                        uint32_t timeout,
                                                        uint8_t flags);

/**
 * Make a synchronous method call from this object
 *
 * @param proxyObj     ProxyBusObject on which to call the method.
 * @param method       The alljoyn_interfacedescription_member specifying the method being invoked.
 * @param args         The arguments for the method call (can be NULL)
 * @param numArgs      The number of arguments
 * @param replyMsg     The reply message received for the method call
 * @param timeout      Timeout specified in milliseconds to wait for a reply.
 *                     Recommended default #ALLJOYN_MESSAGE_DEFAULT_TIMEOUT which is 25000 ms.
 * @param flags        Logical OR of the message flags for this method call. The following flags apply to method calls:
 *                     - If #ALLJOYN_MESSAGE_FLAG_ENCRYPTED is set the message is authenticated and the payload if any is encrypted.
 *                     - If #ALLJOYN_MESSAGE_FLAG_COMPRESSED is set the header is compressed for destinations that can handle header compression.
 *                     - If #ALLJOYN_MESSAGE_FLAG_AUTO_START is set the bus will attempt to start a service if it is not running.
 *                     Set value to '0' for no flags.
 *
 *
 * @return
 *      - #ER_OK if the method call succeeded and the reply message type is #MESSAGE_METHOD_RET
 *      - #ER_BUS_REPLY_IS_ERROR_MESSAGE if the reply message type is #MESSAGE_ERROR
 */
extern AJ_API QStatus alljoyn_proxybusobject_methodcall_member(alljoyn_proxybusobject proxyObj,
                                                               const alljoyn_interfacedescription_member method,
                                                               const alljoyn_msgarg args,
                                                               size_t numArgs,
                                                               alljoyn_message replyMsg,
                                                               uint32_t timeout,
                                                               uint8_t flags);

/**
 * Make a fire-and-forget method call from this object. The caller will not be able to tell if
 * the method call was successful or not. This is equivalent to calling MethodCall() with
 * flags == ALLJOYN_FLAG_NO_REPLY_EXPECTED. Because this call doesn't block it can be made from
 * within a signal handler.
 *
 * @param proxyObj     ProxyBusObject on which to call the method.
 * @param ifaceName    Name of interface.
 * @param methodName   Name of method.
 * @param args         The arguments for the method call (can be NULL)
 * @param numArgs      The number of arguments
 * @param flags        Logical OR of the message flags for this method call. The following flags apply to method calls:
 *                     - If #ALLJOYN_FLAG_ENCRYPTED is set the message is authenticated and the payload if any is encrypted.
 *                     - If #ALLJOYN_FLAG_COMPRESSED is set the header is compressed for destinations that can handle header compression.
 *                     - If #ALLJOYN_FLAG_AUTO_START is set the bus will attempt to start a service if it is not running.
 *                     Set value to '0' for no flags.
 *
 * @return
 *      - #ER_OK if the method call succeeded
 */
extern AJ_API QStatus alljoyn_proxybusobject_methodcall_noreply(alljoyn_proxybusobject proxyObj,
                                                                const char* ifaceName,
                                                                const char* methodName,
                                                                const alljoyn_msgarg args,
                                                                size_t numArgs,
                                                                uint8_t flags);

/**
 * Make a fire-and-forget method call from this object. The caller will not be able to tell if
 * the method call was successful or not. This is equivalent to calling MethodCall() with
 * flags == ALLJOYN_FLAG_NO_REPLY_EXPECTED. Because this call doesn't block it can be made from
 * within a signal handler.
 *
 * @param proxyObj     ProxyBusObject on which to call the method.
 * @param method       Method being invoked.
 * @param args         The arguments for the method call (can be NULL)
 * @param numArgs      The number of arguments
 * @param flags        Logical OR of the message flags for this method call. The following flags apply to method calls:
 *                     - If #ALLJOYN_FLAG_ENCRYPTED is set the message is authenticated and the payload if any is encrypted.
 *                     - If #ALLJOYN_FLAG_COMPRESSED is set the header is compressed for destinations that can handle header compression.
 *                     - If #ALLJOYN_FLAG_AUTO_START is set the bus will attempt to start a service if it is not running.
 *                     Set value to '0' for no flags.
 *
 * @return
 *      - #ER_OK if the method call succeeded
 */
extern AJ_API QStatus alljoyn_proxybusobject_methodcall_member_noreply(alljoyn_proxybusobject proxyObj,
                                                                       const alljoyn_interfacedescription_member method,
                                                                       const alljoyn_msgarg args,
                                                                       size_t numArgs,
                                                                       uint8_t flags);

/**
 * Make an asynchronous method call from this object
 *
 * @param proxyObj     ProxyBusObject on which to call the method
 * @param ifaceName    Name of interface for method.
 * @param methodName   Name of method.
 * @param replyFunc    The function that is called to deliver the reply
 * @param args         The arguments for the method call (can be NULL)
 * @param numArgs      The number of arguments (can be 0)
 * @param context      User-defined context that will be returned to the reply handler
 *                     Recommended default NULL.
 * @param timeout      Timeout specified in milliseconds to wait for a reply
 *                     Recommended default #ALLJOYN_MESSAGE_DEFAULT_TIMEOUT which is 25000 ms
 * @param flags        Logical OR of the message flags for this method call. The following flags apply to method calls:
 *                     - If #ALLJOYN_FLAG_ENCRYPTED is set the message is authenticated and the payload if any is encrypted.
 *                     - If #ALLJOYN_FLAG_COMPRESSED is set the header is compressed for destinations that can handle header compression.
 *                     - If #ALLJOYN_FLAG_AUTO_START is set the bus will attempt to start a service if it is not running.
 *                     Set value to '0' for no flags.
 * @return
 *      - ER_OK if successful
 *      - An error status otherwise
 */
extern AJ_API QStatus alljoyn_proxybusobject_methodcallasync(alljoyn_proxybusobject proxyObj,
                                                             const char* ifaceName,
                                                             const char* methodName,
                                                             alljoyn_messagereceiver_replyhandler_ptr replyFunc,
                                                             const alljoyn_msgarg args,
                                                             size_t numArgs,
                                                             void* context,
                                                             uint32_t timeout,
                                                             uint8_t flags);

/**
 * Make an asynchronous method call from this object
 *
 * @param proxyObj     ProxyBusObject on which to call the method
 * @param method       Method being invoked.
 * @param replyFunc    The function that is called to deliver the reply
 * @param args         The arguments for the method call (can be NULL)
 * @param numArgs      The number of arguments (can be 0)
 * @param context      User-defined context that will be returned to the reply handler
 *                     Recommended default NULL.
 * @param timeout      Timeout specified in milliseconds to wait for a reply
 *                     Recommended default #ALLJOYN_MESSAGE_DEFAULT_TIMEOUT which is 25000 ms
 * @param flags        Logical OR of the message flags for this method call. The following flags apply to method calls:
 *                     - If #ALLJOYN_FLAG_ENCRYPTED is set the message is authenticated and the payload if any is encrypted.
 *                     - If #ALLJOYN_FLAG_COMPRESSED is set the header is compressed for destinations that can handle header compression.
 *                     - If #ALLJOYN_FLAG_AUTO_START is set the bus will attempt to start a service if it is not running.
 *                     Set value to '0' for no flags.
 * @return
 *      - ER_OK if successful
 *      - An error status otherwise
 */
extern AJ_API QStatus alljoyn_proxybusobject_methodcallasync_member(alljoyn_proxybusobject proxyObj,
                                                                    const alljoyn_interfacedescription_member method,
                                                                    alljoyn_messagereceiver_replyhandler_ptr replyFunc,
                                                                    const alljoyn_msgarg args,
                                                                    size_t numArgs,
                                                                    void* context,
                                                                    uint32_t timeout,
                                                                    uint8_t flags);

/**
 * Initialize this proxy object from an XML string. Calling this method does several things:
 *
 *  -# Create and register any new InterfaceDescription(s) that are mentioned in the XML.
 *     (Interfaces that are already registered with the bus are left "as-is".)
 *  -# Add all the interfaces mentioned in the introspection data to this ProxyBusObject.
 *  -# Recursively create any child ProxyBusObject(s) and create/add their associated @n
 *     interfaces as mentioned in the XML. Then add the descendant object(s) to the appropriate
 *     descendant of this ProxyBusObject (in the children collection). If the named child object
 *     already exists as a child of the appropriate ProxyBusObject, then it is updated
 *     to include any new interfaces or children mentioned in the XML.
 *
 * Note that when this method fails during parsing, the return code will be set accordingly.
 * However, any interfaces which were successfully parsed prior to the failure
 * may be registered with the bus. Similarly, any objects that were successfully created
 * before the failure will exist in this object's set of children.
 *
 * @param proxyObj    ProxyBusObject which will be initialized from the XML string
 * @param xml         An XML string in DBus introspection format.
 * @param identifier  An optional identifying string to include in error logging messages.
 *
 * @return
 *      - #ER_OK if parsing is completely successful.
 *      - An error status otherwise.
 */
extern AJ_API QStatus alljoyn_proxybusobject_parsexml(alljoyn_proxybusobject proxyObj, const char* xml, const char* identifier);

/**
 * Explicitly secure the connection to the remote peer for this proxy object. Peer-to-peer
 * connections can only be secured if EnablePeerSecurity() was previously called on the bus
 * attachment for this proxy object. If the peer-to-peer connection is already secure this
 * function does nothing. Note that peer-to-peer connections are automatically secured when a
 * method call or signal requiring encryption is sent or received.
 *
 * This call causes messages to be send on the bus, therefore it cannot be called within AllJoyn
 * callbacks (method/signal/reply handlers or ObjectRegistered callbacks, etc.)
 *
 * @param proxyObj   ProxyBusObject to explicitly set up a secure a connection
 * @param forceAuth  If true, forces an re-authentication even if the peer connection is already
 *                   authenticated.
 *                   Recommended default QCC_FALSE
 *
 * @return
 *          - #ER_OK if the connection was secured or an error status indicating that the
 *            connection could not be secured.
 *          - #ER_BUS_NO_AUTHENTICATION_MECHANISM if BusAttachment::EnablePeerSecurity() has not been called.
 *          - #ER_AUTH_FAIL if the attempt(s) to authenticate the peer failed.
 *          - Other error status codes indicating a failure.
 */
extern AJ_API QStatus alljoyn_proxybusobject_secureconnection(alljoyn_proxybusobject proxyObj, QCC_BOOL forceAuth);

/**
 * Asynchronously secure the connection to the remote peer for this proxy object. Peer-to-peer
 * connections can only be secured if EnablePeerSecurity() was previously called on the bus
 * attachment for this proxy object. If the peer-to-peer connection is already secure this
 * function does nothing. Note that peer-to-peer connections are automatically secured when a
 * method call or signal requiring encryption is sent or received.
 *
 * Notification of success or failure is via the AuthListener passed to EnablePeerSecurity().
 *
 * @param proxyObj   ProxyBusObject to explicitly set up a secure a connection
 * @param forceAuth  If true, forces an re-authentication even if the peer connection is already
 *                   authenticated.
 *                   Recommended default QCC_FALSE
 *
 * @return
 *          - #ER_OK if securing could begin.
 *          - #ER_BUS_NO_AUTHENTICATION_MECHANISM if BusAttachment::EnablePeerSecurity() has not been called.
 *          - Other error status codes indicating a failure.
 */
extern AJ_API QStatus alljoyn_proxybusobject_secureconnectionasync(alljoyn_proxybusobject proxyObj, QCC_BOOL forceAuth);

/**
 * Returns a pointer to an interface description. Returns NULL if the object does not implement
 * the requested interface.
 *
 * @param proxyObj The proxy bus object to obtain an interface description from.
 * @param iface  The name of interface to get.
 *
 * @return
 *      - A pointer to the requested interface description.
 *      - NULL if requested interface is not implemented or not found
 */
extern AJ_API const alljoyn_interfacedescription alljoyn_proxybusobject_getinterface(alljoyn_proxybusobject proxyObj, const char* iface);

/**
 * Returns the interfaces implemented by this object. Note that all proxy bus objects
 * automatically inherit the "org.freedesktop.DBus.Peer" which provides the built-in "ping"
 * method, so this method always returns at least that one interface.
 *
 * @param proxyObj The proxy bus object to obtain an interface descriptions from.
 * @param ifaces     A pointer to an InterfaceDescription array to receive the interfaces. Can be NULL in
 *                   which case no interfaces are returned and the return value gives the number
 *                   of interface available.
 * @param numIfaces  The size of the InterfaceDescription array. If this value is smaller than the total
 *                   number of interfaces only numIfaces will be returned.
 *                   (DEFAULT) if numInterfaces is 0 then an array of all the interfaces will be returned.
 *
 * @return  The number of interfaces returned or the total number of interfaces if ifaces is NULL.
 */
extern AJ_API size_t alljoyn_proxybusobject_getinterfaces(alljoyn_proxybusobject proxyObj, const alljoyn_interfacedescription* ifaces, size_t numIfaces);

/**
 * Return the absolute object path for the remote object.
 *
 * @param proxyObj  the proxy bus object to read the object path from.
 *
 * @return Object path
 */
extern AJ_API const char* alljoyn_proxybusobject_getpath(alljoyn_proxybusobject proxyObj);

/**
 * Return the remote service name for this object.
 *
 * @param proxyObj  the proxy bus object to read the service name (Bus Name) path from.
 *
 * @return Service name (typically a well-known service name but may be a unique name)
 */
extern AJ_API const char* alljoyn_proxybusobject_getservicename(alljoyn_proxybusobject proxyObj);

/**
 * Return the session Id for this object.
 *
 * @param proxyObj the proxy bus object we wish to obtain the session ID from.
 * @return Session Id
 */
extern AJ_API alljoyn_sessionid alljoyn_proxybusobject_getsessionid(alljoyn_proxybusobject proxyObj);

/**
 * Tests if this object implements the requested interface.
 *
 * @param proxyObj the proxy bus object we wish check for the interface
 * @param iface  The interface to check
 *
 * @return  true if the object implements the requested interface
 */
extern AJ_API QCC_BOOL alljoyn_proxybusobject_implementsinterface(alljoyn_proxybusobject proxyObj, const char* iface);

/**
 * create a copy of a proxybusobject.  This will create a new alljoyn_proxybusobject and
 * must be cleaned up using alljoyn_proxybusobject_destroy.
 *
 * @param source      the alljoyn_proxybusobject to be copied
 *
 * @return copy of the alljoyn_proxybusobject argument is returned
 */
extern AJ_API alljoyn_proxybusobject alljoyn_proxybusobject_copy(const alljoyn_proxybusobject source);

/**
 * Indicates if this is a valid (usable) proxy bus object.
 *
 * @return true if a valid proxy bus object, false otherwise.
 */
extern AJ_API QCC_BOOL alljoyn_proxybusobject_isvalid(alljoyn_proxybusobject proxyObj);

#if 0
/*TODO create C bindings for the following C++ methods */

/**
 * Returns an array of ProxyBusObjects for the children of this %ProxyBusObject.
 *
 * @param children     A pointer to an %ProxyBusObject array to receive the children. Can be NULL in
 *                     which case no children are returned and the return value gives the number
 *                     of children available.
 * @param numChildren  The size of the %ProxyBusObject array. If this value is smaller than the total
 *                     number of children only numChildren will be returned.
 *
 * @return  The number of children returned or the total number of children if children is NULL.
 */
size_t GetChildren(ProxyBusObject** children = NULL, size_t numChildren = 0);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
