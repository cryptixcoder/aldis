/**
 * @file BusAttachment.h is the top-level object responsible for connecting to a
 * message bus.
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
#ifndef _ALLJOYN_C_BUSATTACHMENT_H
#define _ALLJOYN_C_BUSATTACHMENT_H

#include <qcc/platform.h>

#include <alljoyn_c/AjAPI.h>
#include <alljoyn_c/KeyStoreListener.h>
#include <alljoyn_c/AuthListener.h>
#include <alljoyn_c/BusListener.h>
#include <alljoyn_c/BusObject.h>
#include <alljoyn_c/ProxyBusObject.h>
#include <alljoyn_c/InterfaceDescription.h>
#include <alljoyn_c/Session.h>
#include <alljoyn_c/SessionListener.h>
#include <alljoyn_c/SessionPortListener.h>
#include <Status.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ALLJOYN_OPAQUE_BUSATTACHMENT_
#define _ALLJOYN_OPAQUE_BUSATTACHMENT_
typedef struct _alljoyn_busattachment_handle*               alljoyn_busattachment;
#endif

/**
 * Type for the joinsession callback used with the asynchronous joinsession request.
 */
typedef void (*alljoyn_busattachment_joinsessioncb_ptr)(QStatus status, alljoyn_sessionid sessionId, const alljoyn_sessionopts opts, void* context);

/**
 * Allocate a BusAttachment.
 *
 * @note Any BusAttachment allocated using this function must be freed using
 *
 * @param applicationName       Name of the application.
 * @param allowRemoteMessages   QCC_TRUE if this attachment is allowed to receive messages from remote devices.
 */
extern AJ_API alljoyn_busattachment alljoyn_busattachment_create(const char* applicationName, QCC_BOOL allowRemoteMessages);

/**
 * Free an allocated BusAttachment.
 *
 * @param bus BusAttachment to free.
 */
extern AJ_API void alljoyn_busattachment_destroy(alljoyn_busattachment bus);

/**
 * @brief Start the process of spinning up the independent threads used in
 * the bus attachment, preparing it for action.
 *
 * This method only begins the process of starting the bus. Sending and
 * receiving messages cannot begin until the bus is Connect()ed.
 *
 * There are two ways to determine whether the bus is currently connected:
 *    -# alljoyn_busattachment() returns QCC_TRUE
 *    -# alljoyn_busobject_callback object_registered is called by the bus
 *
 * In most cases, it is not required to understand the threading model of
 * the bus attachment, with one important exception: The bus attachment may
 * send callbacks to registered listeners using its own internal threads.
 * This means that any time a listener of any kind is used in a program, the
 * implication is that a the overall program is multithreaded, irrespective
 * of whether or not threads are explicitly used.  This, in turn, means that
 * any time shared state is accessed in listener methods, that state must be
 * protected.
 *
 * As soon as Start() is called, clients of a bus attachment with listeners
 * must be prepared to receive callbacks on those listeners in the context
 * of a thread that will be different from the thread running the main
 * program or any other thread in the client.
 *
 * Although intimate knowledge of the details of the threading model are not
 * required to use a bus attachment (beyond the caveat above) we do provide
 * methods on the bus attachment that help users reason about more complex
 * threading situations.  This will apply to situations where clients of the
 * bus attachment are multithreaded and need to interact with the
 * multithreaded bus attachment.  These methods can be especially useful
 * during shutdown, when the two separate threading systems need to be
 * gracefully brought down together.
 *
 * The BusAttachment methods Start(), Stop() and Join() all work together to
 * manage the autonomous activities that can happen in a BusAttachment.
 * These activities are carried out by so-called hardware threads.  POSIX
 * defines functions used to control hardware threads, which it calls
 * pthreads.  Many threading packages use similar constructs.
 *
 * In a threading package, a start method asks the underlying system to
 * arrange for the start of thread execution.  Threads are not necessarily
 * running when the start method returns, but they are being *started*.  Some time later,
 * a thread of execution appears in a thread run function, at which point the
 * thread is considered *running*.  At some later time, executing a stop method asks the
 * underlying system to arrange for a thread to end its execution.  The system
 * typically sends a message to the thread to ask it to stop doing what it is doing.
 * The thread is running until it responds to the stop message, at which time the
 * run method exits and the thread is considered *stopping*.
 *
 * Note that neither of Start() nor Stop() are synchronous in the sense that
 * one has actually accomplished the desired effect upon the return from a
 * call.  Of particular interest is the fact that after a call to Stop(),
 * threads will still be *running* for some non-deterministic time.
 *
 * In order to wait until all of the threads have actually stopped, a
 * blocking call is required.  In threading packages this is typically
 * called join, and our corresponding method is called Join().
 *
 * A Start() method call should be thought of as mapping to a threading
 * package start function.  it causes the activity threads in the
 * BusAttachment to be spun up and gets the attachment ready to do its main
 * job.  As soon as Start() is called, the user should be prepared for one
 * or more of these threads of execution to pop out of the bus attachment
 * and into a listener callback.
 *
 * The Stop() method call should be thought of as mapping to a threading
 * package stop function.  It asks the BusAttachment to begin shutting down
 * its various threads of execution, but does not wait for any threads to exit.
 *
 * A call to the Join() method should be thought of as mapping to a
 * threading package join function call.  It blocks and waits until all of
 * the threads in the BusAttachment have in fact exited their Run functions,
 * gone through the stopping state and have returned their status.  When
 * the Join() method returns, one may be assured that no threads are running
 * in the bus attachment, and therefore there will be no callbacks in
 * progress and no further callbacks will ever come out of a particular
 * instance of a bus attachment.
 *
 * It is important to understand that since Start(), Stop() and Join() map
 * to threads concepts and functions, one should not expect them to clean up
 * any bus attachment state when they are called.  These functions are only
 * present to help in orderly termination of complex threading systems.
 *
 * @see alljoyn_busattachment_stop()
 * @see alljoyn_busattachment_join()
 *
 * @param bus The BusAttachment to start.
 *
 * @return
 *      - #ER_OK if successful.
 *      - #ER_BUS_BUS_ALREADY_STARTED if already started
 *      - Other error status codes indicating a failure
 */
extern AJ_API QStatus alljoyn_busattachment_start(alljoyn_busattachment bus);

/**
 * @brief Ask the threading subsystem in the bus attachment to begin the
 * process of ending the execution of its threads.
 *
 * The Stop() method call on a bus attachment should be thought of as
 * mapping to a threading package stop function.  It asks the BusAttachment
 * to begin shutting down its various threads of execution, but does not
 * wait for any threads to exit.
 *
 * A call to Stop() is implied as one of the first steps in the destruction
 * of a bus attachment.
 *
 * @warning There is no guarantee that a listener callback may begin executing
 * after a call to Stop().  To achieve that effect, the Stop() must be followed
 * by a Join().
 *
 * @see alljoyn_busattachment_start()
 * @see alljoyn_busattachment_join()
 *
 * @param bus                 BusAttachment to stop.
 *
 * @return
 *      - #ER_OK if successful.
 *      - An error status if unable to stop the message bus
 */
extern AJ_API QStatus alljoyn_busattachment_stop(alljoyn_busattachment bus);

/**
 * @brief Wait for all of the threads spawned by the bus attachment to be
 * completely exited.
 *
 * A call to the Join() method should be thought of as mapping to a
 * threading package join function call.  It blocks and waits until all of
 * the threads in the BusAttachment have, in fact, exited their Run functions,
 * gone through the stopping state and have returned their status.  When
 * the Join() method returns, one may be assured that no threads are running
 * in the bus attachment, and therefore there will be no callbacks in
 * progress and no further callbacks will ever come out of the instance of a
 * bus attachment on which Join() was called.
 *
 * A call to Join() is implied as one of the first steps in the destruction
 * of a bus attachment.  Thus, when a bus attachment is destroyed, it is
 * guaranteed that before it completes its destruction process, there will be
 * no callbacks in process.
 *
 * @warning If Join() is called without a previous Stop() it will result in
 * blocking "forever."
 *
 * @see alljoyn_busattachment_start()
 * @see alljoyn_busattachment_stop()
 *
 * @param bus BusAttachment to join.
 *
 * @return
 *      - #ER_OK if successful.
 *      - #ER_BUS_BUS_ALREADY_STARTED if already started
 *      - Other error status codes indicating a failure
 */
extern AJ_API QStatus alljoyn_busattachment_join(alljoyn_busattachment bus);
/**
 * Create an interface description with a given name.
 *
 * @param bus    The BusAttachment on which to create an interface.
 * @param name   The requested interface name.
 * @param[out] iface
 *      - Interface description
 *      - NULL if cannot be created.
 * @param secure If QCC_TRUE the interface is secure and method calls and signals will be encrypted.
 *
 * @return
 *      - #ER_OK if creation was successful.
 *      - #ER_BUS_IFACE_ALREADY_EXISTS if requested interface already exists
 * @see BusAttachment::CreateInterface
 */
extern AJ_API QStatus alljoyn_busattachment_createinterface(alljoyn_busattachment bus, const char* name,
                                                            alljoyn_interfacedescription* iface, QCC_BOOL secure);

/**
 * Connect to a remote bus address.
 *
 * @param bus          The BusAttachment to be connected.
 * @param connectSpec  A transport connection spec string of the form:
 *                     @c "<transport>:<param1>=<value1>,<param2>=<value2>...[;]"
 *
 * @return
 *      - #ER_OK if successful.
 *      - An error status otherwise
 */
extern AJ_API QStatus alljoyn_busattachment_connect(alljoyn_busattachment bus, const char* connectSpec);

/**
 * Register an object that will receive bus event notifications.
 *
 * @param bus       The BusAttachment on which to attach a BusListener.
 * @param listener  Object instance that will receive bus event notifications.
 */
extern AJ_API void alljoyn_busattachment_registerbuslistener(alljoyn_busattachment bus, alljoyn_buslistener listener);

/**
 * Unregister an object that was previously registered with RegisterBusListener.
 *
 * @param bus       The BusAttachment from which to detach a BusListener.
 * @param listener  Object instance to un-register as a listener.
 */
extern AJ_API void alljoyn_busattachment_unregisterbuslistener(alljoyn_busattachment bus, alljoyn_buslistener listener);

/**
 * Register interest in a well-known name prefix for the purpose of discovery.
 * This method is a shortcut/helper that issues an org.alljoyn.Bus.FindAdvertisedName method call to the local daemon
 * and interprets the response.
 *
 * @param      bus           The BusAttachment on which to register interest in the namePrefix.
 * @param[in]  namePrefix    Well-known name prefix that application is interested in receiving
 *                           BusListener::FoundAdvertisedName notifications about.
 *
 * @return
 *      - #ER_OK iff daemon response was received and discovery was successfully started.
 *      - #ER_BUS_NOT_CONNECTED if a connection has not been made with a local bus.
 *      - Other error status codes indicating a failure.
 */
extern AJ_API QStatus alljoyn_busattachment_findadvertisedname(alljoyn_busattachment bus, const char* namePrefix);

/**
 * Cancel interest in a well-known name prefix that was previously
 * registered with FindAdvertisedName.  This method is a shortcut/helper
 * that issues an org.alljoyn.Bus.CancelFindAdvertisedName method
 * call to the local daemon and interprets the response.
 *
 * @param      bus           The BusAttachment from which to remove interest in the namePrefix.
 * @param[in]  namePrefix    Well-known name prefix that application is no longer interested in receiving
 *                           BusListener::FoundAdvertisedName notifications about.
 *
 * @return
 *      - #ER_OK iff daemon response was received and cancel was successfully completed.
 *      - #ER_BUS_NOT_CONNECTED if a connection has not been made with a local bus.
 *      - Other error status codes indicating a failure.
 */
extern AJ_API QStatus alljoyn_busattachment_cancelfindadvertisedname(alljoyn_busattachment bus, const char* namePrefix);

/**
 * Advertise the existence of a well-known name to other (possibly disconnected) AllJoyn daemons.
 *
 * This method is a shortcut/helper that issues an org.alljoyn.Bus.AdvertisedName method call to the local daemon
 * and interprets the response.
 *
 * @param[in]  bus           The bus on which to advertise the name.
 * @param[in]  name          the well-known name to advertise. (Must be owned by the caller via RequestName).
 * @param[in]  transports    Set of transports to use for sending advertisment.
 *
 * @return
 *      - #ER_OK iff daemon response was received and advertise was successful.
 *      - #ER_BUS_NOT_CONNECTED if a connection has not been made with a local bus.
 *      - Other error status codes indicating a failure.
 */
extern AJ_API QStatus alljoyn_busattachment_advertisename(alljoyn_busattachment bus, const char* name, alljoyn_transportmask transports);

/**
 * Stop advertising the existence of a well-known name to other AllJoyn daemons.
 *
 * This method is a shortcut/helper that issues an org.alljoyn.Bus.CancelAdvertiseName method call to the local daemon
 * and interprets the response.
 *
 * @param[in]  name          A well-known name that was previously advertised via AdvertiseName.
 * @param[in]  transports    Set of transports whose name advertisment will be cancelled.
 *
 * @return
 *      - #ER_OK iff daemon response was received and advertisements were sucessfully stopped.
 *      - #ER_BUS_NOT_CONNECTED if a connection has not been made with a local bus.
 *      - Other error status codes indicating a failure.
 */
extern AJ_API QStatus alljoyn_busattachment_canceladvertisename(alljoyn_busattachment bus, const char* name, alljoyn_transportmask transports);

/**
 * Retrieve an existing activated InterfaceDescription.
 *
 * @param bus        The BusAttachment from which to retrieve the interface.
 * @param name       Interface name.
 *
 * @return
 *      - A pointer to the registered interface
 *      - NULL if interface doesn't exist
 */
extern AJ_API const alljoyn_interfacedescription alljoyn_busattachment_getinterface(alljoyn_busattachment bus, const char* name);

/**
 * Join a session.
 * This method is a shortcut/helper that issues an org.alljoyn.Bus.JoinSession method call to the local daemon
 * and interprets the response.
 *
 * @param[in]  bus              BusAttachment with which to join a session.
 * @param[in]  sessionHost      Bus name of attachment that is hosting the session to be joined.
 * @param[in]  sessionPort      SessionPort of sessionHost to be joined.
 * @param[in]  listener         Optional listener called when session related events occur. May be NULL.
 * @param[out] sessionId        Unique identifier for session.
 * @param[in,out] opts          Session options.
 *
 * @return
 *      - #ER_OK iff daemon response was received and the session was successfully joined.
 *      - #ER_BUS_NOT_CONNECTED if a connection has not been made with a local bus.
 *      - Other error status codes indicating a failure.
 */
extern AJ_API QStatus alljoyn_busattachment_joinsession(alljoyn_busattachment bus, const char* sessionHost,
                                                        alljoyn_sessionport sessionPort, alljoyn_sessionlistener listener,
                                                        alljoyn_sessionid* sessionId, alljoyn_sessionopts opts);


/**
 * Join a session.
 * This method is a shortcut/helper that issues an org.alljoyn.Bus.JoinSession method call to the local daemon
 * and interprets the response.
 *
 * This call executes asynchronously. When the JoinSession response is received, the callback will be called.
 *
 * @param[in]  bus              BusAttachment with which to join a session.
 * @param[in]  sessionHost      Bus name of attachment that is hosting the session to be joined.
 * @param[in]  sessionPort      SessionPort of sessionHost to be joined.
 * @param[in]  listener         Optional listener called when session related events occur. May be NULL.
 * @param[in]  opts             Session options.
 * @param[in]  callback         Called when JoinSession response is received.
 * @param[in]  context          User defined context which will be passed as-is to callback.
 *
 * @return
 *      - #ER_OK iff method call to local daemon response was was successful.
 *      - #ER_BUS_NOT_CONNECTED if a connection has not been made with a local bus.
 *      - Other error status codes indicating a failure.
 */
extern AJ_API QStatus alljoyn_busattachment_joinsessionasync(alljoyn_busattachment bus,
                                                             const char* sessionHost,
                                                             alljoyn_sessionport sessionPort,
                                                             alljoyn_sessionlistener listener,
                                                             const alljoyn_sessionopts opts,
                                                             alljoyn_busattachment_joinsessioncb_ptr callback,
                                                             void* context);

/**
 * Register a BusObject
 *
 * @param bus      The bus on which to register the object.
 * @param obj      BusObject to register.
 *
 * @return
 *      - #ER_OK if successful.
 *      - #ER_BUS_BAD_OBJ_PATH for a bad object path
 */
extern AJ_API QStatus alljoyn_busattachment_registerbusobject(alljoyn_busattachment bus, alljoyn_busobject obj);

/**
 * Unregister a BusObject
 *
 * @param bus     The bus from which to unregister the object.
 * @param object  Object to be unregistered.
 */
extern AJ_API void alljoyn_busattachment_unregisterbusobject(alljoyn_busattachment bus, alljoyn_busobject object);

/**
 * Request a well-known name.
 * This method is a shortcut/helper that issues an org.freedesktop.DBus.RequestName method call to the local daemon
 * and interprets the response.
 *
 * @param[in]  bus            The bus on which to request the specified name.
 * @param[in]  requestedName  Well-known name being requested.
 * @param[in]  flags          Bitmask of DBUS_NAME_FLAG_* defines (see DBusStd.h)
 *
 * @return
 *      - #ER_OK iff daemon response was received and request was successful.
 *      - #ER_BUS_NOT_CONNECTED if a connection has not been made with a local bus.
 *      - Other error status codes indicating a failure.
 */
extern AJ_API QStatus alljoyn_busattachment_requestname(alljoyn_busattachment bus, const char* requestedName, uint32_t flags);

/**
 * Release a previously requeted well-known name.
 * This method is a shortcut/helper that issues an org.freedesktop.DBus.ReleaseName method call to the local daemon
 * and interprets the response.
 *
 * @parma[in]  bus           The bus from which to release the name.
 * @param[in]  name          Well-known name being released.
 *
 * @return
 *      - #ER_OK iff daemon response was received amd the name was successfully released.
 *      - #ER_BUS_NOT_CONNECTED if a connection has not been made with a local bus.
 *      - Other error status codes indicating a failure.
 */
extern AJ_API QStatus alljoyn_busattachment_releasename(alljoyn_busattachment bus, const char* name);

/**
 * Make a SessionPort available for external BusAttachments to join.
 *
 * Each BusAttachment binds its own set of SessionPorts. Session joiners use the bound session
 * port along with the name of the attachement to create a persistent logical connection (called
 * a Session) with the original BusAttachment.
 *
 * A SessionPort and bus name form a unique identifier that BusAttachments use when joining a
 * session.
 *
 * SessionPort values can be pre-arranged between AllJoyn services and their clients (well-known
 * SessionPorts).
 *
 * Once a session is joined using one of the service's well-known SessionPorts, the service may
 * bind additional SessionPorts (dyanamically) and share these SessionPorts with the joiner over
 * the original session. The joiner can then create additional sessions with the service by
 * calling JoinSession with these dynamic SessionPort ids.
 *
 * @param[in]     bus              The bus on which to make the session port available.
 * @param[in,out] sessionPort      SessionPort value to bind or SESSION_PORT_ANY to allow this method
 *                                 to choose an available port. On successful return, this value
 *                                 contains the chosen SessionPort.
 *
 * @param[in]     opts             Session options that joiners must agree to in order to
 *                                 successfully join the session.
 *
 * @param[in]     listener  Called by the bus when session related events occur.
 *
 * @return
 *      - #ER_OK iff daemon response was received and the bind operation was successful.
 *      - #ER_BUS_NOT_CONNECTED if a connection has not been made with a local bus.
 *      - Other error status codes indicating a failure.
 */
extern AJ_API QStatus alljoyn_busattachment_bindsessionport(alljoyn_busattachment bus, alljoyn_sessionport* sessionPort,
                                                            const alljoyn_sessionopts opts, alljoyn_sessionportlistener listener);

/**
 * Cancel an existing port binding.
 *
 * @param[in]   bus            The bus on which to cancel the port binding.
 * @param[in]   sessionPort    Existing session port to be un-bound.
 *
 * @return
 *      - #ER_OK iff daemon response was received and the bind operation was successful.
 *      - #ER_BUS_NOT_CONNECTED if a connection has not been made with a local bus.
 *      - Other error status codes indicating a failure.
 */
extern AJ_API QStatus alljoyn_busattachment_unbindsessionport(alljoyn_busattachment bus, alljoyn_sessionport sessionPort);

/**
 * Enable peer-to-peer security. This function must be called by applications that want to use
 * authentication and encryption . The bus must have been started by calling
 * BusAttachment::Start() before this function is called. If the application is providing its
 * own key store implementation it must have already called RegisterKeyStoreListener() before
 * calling this function.
 *
 * @param bus              The bus on which to enable security.
 * @param authMechanisms   The authentication mechanism(s) to use for peer-to-peer authentication.
 *                         If this parameter is NULL peer-to-peer authentication is disabled.
 *
 * @param listener         Passes password and other authentication related requests to the application.
 *
 * @param keyStoreFileName Optional parameter to specify the filename of the default key store. The
 *                         default value is the applicationName parameter of BusAttachment().
 *                         Note that this parameter is only meaningful when using the default
 *                         key store implementation.
 *
 * @param isShared         optional parameter that indicates if the key store is shared between multiple
 *                         applications. It is generally harmless to set this to true even when the
 *                         key store is not shared but it adds some unnecessary calls to the key store
 *                         listener to load and store the key store in this case.
 *
 * @return
 *      - #ER_OK if peer security was enabled.
 *      - #ER_BUS_BUS_NOT_STARTED BusAttachment::Start has not be called
 */
extern AJ_API QStatus alljoyn_busattachment_enablepeersecurity(alljoyn_busattachment bus, const char* authMechanisms,
                                                               alljoyn_authlistener listener, const char* keyStoreFileName,
                                                               QCC_BOOL isShared);

/**
 * Check is peer security has been enabled for this bus attachment.
 *
 * @param bus The bus on which to query if peer security is enabled.
 *
 * @return   Returns QCC_TRUE if peer security has been enabled, QCC_FALSE otherwise.
 */
extern AJ_API QCC_BOOL alljoyn_busattachment_ispeersecurityenabled(alljoyn_busattachment bus);

/**
 * Initialize one more interface descriptions from an XML string in DBus introspection format.
 * The root tag of the XML can be a \<node\> or a standalone \<interface\> tag. To initialize more
 * than one interface the interfaces need to be nested in a \<node\> tag.
 *
 * Note that when this method fails during parsing, the return code will be set accordingly.
 * However, any interfaces which were successfully parsed prior to the failure may be registered
 * with the bus.
 *
 * @param bus     The bus on which to create interfaces.
 * @param xml     An XML string in DBus introspection format.
 *
 * @return
 *      - #ER_OK if parsing is completely successful.
 *      - An error status otherwise.
 */
extern AJ_API QStatus alljoyn_busattachment_createinterfacesfromxml(alljoyn_busattachment bus, const char* xml);

/**
 * Returns the existing activated InterfaceDescriptions.
 *
 * @param ifaces     A pointer to an InterfaceDescription array to receive the interfaces. Can be NULL in
 *                   which case no interfaces are returned and the return value gives the number
 *                   of interface available.
 * @param numIfaces  The size of the InterfaceDescription array. If this value is smaller than the total
 *                   number of interfaces only numIfaces will be returned.
 *
 * @return  The number of interfaces returned or the total number of interfaces if ifaces is NULL.
 */
extern AJ_API size_t alljoyn_busattachment_getinterfaces(const alljoyn_busattachment bus,
                                                         const alljoyn_interfacedescription* ifaces, size_t numIfaces);

/**
 * Delete an interface description with a given name.
 *
 * Deleting an interface is only allowed if that interface has never been activated.
 *
 * @param bus    The bus from which to delete the interface.
 * @param iface  The un-activated interface to be deleted.
 *
 * @return
 *      - #ER_OK if deletion was successful
 *      - #ER_BUS_NO_SUCH_INTERFACE if interface was not found
 */
extern AJ_API QStatus alljoyn_busattachment_deleteinterface(alljoyn_busattachment bus, alljoyn_interfacedescription iface);
/**
 * Returns QCC_TRUE if the mesage bus has been started.
 *
 * @param bus The bus to query.
 */
extern AJ_API QCC_BOOL alljoyn_busattachment_isstarted(alljoyn_busattachment bus);

/**
 * Returns QCC_TRUE if the mesage bus has been requested to stop.
 *
 * @param bus The bus to query.
 */
extern AJ_API QCC_BOOL alljoyn_busattachment_isstopping(alljoyn_busattachment bus);

/**
 * Indicate whether bus is currently connected.
 *
 * Messages can only be sent or received when the bus is connected.
 *
 * @param bus The bus to query.
 * @return true if the bus is connected.
 */
extern AJ_API QCC_BOOL alljoyn_busattachment_isconnected(const alljoyn_busattachment bus);

/**
 * Disconnect a remote bus address connection.
 *
 * @param bus          The bus to disconnect.
 * @param connectSpec  The transport connection spec used to connect.
 *
 * @return
 *          - #ER_OK if successful
 *          - #ER_BUS_BUS_NOT_STARTED if the bus is not started
 *          - #ER_BUS_NOT_CONNECTED if the %BusAttachment is not connected to the bus
 *          - Other error status codes indicating a failure
 */
extern AJ_API QStatus alljoyn_busattachment_disconnect(alljoyn_busattachment bus, const char* connectSpec);

/**
 * Get the org.freedesktop.DBus proxy object.
 *
 * @param bus The bus from which to get the object.
 *
 * @return org.freedesktop.DBus proxy object
 */
extern AJ_API const alljoyn_proxybusobject alljoyn_busattachment_getdbusproxyobj(alljoyn_busattachment bus);

/**
 * Get the org.alljoyn.Bus proxy object.
 *
 * @param bus The bus from which to get the object.
 *
 * @return org.alljoyn.Bus proxy object
 */
extern AJ_API const alljoyn_proxybusobject alljoyn_busattachment_getalljoynproxyobj(alljoyn_busattachment bus);

/**
 * Get the org.alljoyn.Debug proxy object.
 *
 * @param bus The bus from which to get the object.
 *
 * @return org.alljoyn.Debug proxy object
 */
extern AJ_API const alljoyn_proxybusobject alljoyn_busattachment_getalljoyndebugobj(alljoyn_busattachment bus);

/**
 * Get the unique name of this BusAttachment.
 *
 * @param bus The bus to query.
 *
 * @return The unique name of this BusAttachment.
 */
extern AJ_API const char* alljoyn_busattachment_getuniquename(const alljoyn_busattachment bus);

/**
 * Get the guid of the local daemon as a string
 *
 * @param bus The bus to query.
 *
 * @return GUID of local AllJoyn daemon as a string.
 */
extern AJ_API const char* alljoyn_busattachment_getglobalguidstring(const alljoyn_busattachment bus);


/**
 * Register a signal handler.
 *
 * Signals are forwarded to the signalHandler if sender, interface, member and path
 * qualifiers are ALL met.
 *
 * @param bus            The BusAttachment to register the signal handler with
 * @param receiver       The object receiving the signal.
 * @param signalHandler  The signal handler method.
 * @param member         The interface/member of the signal.
 * @param srcPath        The object path of the emitter of the signal or NULL for all paths.
 * @return #ER_OK
 */
/*
 * may need to create another registersignalhandler that takes a MessageReceiver
 * type.
 */
extern AJ_API QStatus alljoyn_busattachment_registersignalhandler(alljoyn_busattachment bus,
                                                                  alljoyn_messagereceiver_signalhandler_ptr signal_handler,
                                                                  const alljoyn_interfacedescription_member member,
                                                                  const char* srcPath);

/**
 * Unregister a signal handler.
 *
 * Remove the signal handler that was registered with the given parameters.
 *
 * @param bus            The BusAttachment to unregister the signal handler with
 * @param receiver       The object receiving the signal.
 * @param signalHandler  The signal handler method.
 * @param member         The interface/member of the signal.
 * @param srcPath        The object path of the emitter of the signal or NULL for all paths.
 * @return #ER_OK
 */
extern AJ_API QStatus alljoyn_busattachment_unregistersignalhandler(alljoyn_busattachment bus,
                                                                    alljoyn_messagereceiver_signalhandler_ptr signal_handler,
                                                                    const alljoyn_interfacedescription_member member,
                                                                    const char* srcPath);

/**
 * Unregister all signal and reply handlers for the specified alljoyn_busobject.
 *
 * @param bus            The BusAttachment to unregister the signal handler with
 * @param receiver       The message receiver that is being unregistered.
 * @return ER_OK if successful.
 */
extern AJ_API QStatus alljoyn_busattachment_unregisterallhandlers(alljoyn_busattachment bus);
/**
 * Set a key store listener to listen for key store load and store requests.
 * This overrides the internal key store listener.
 *
 * @param bus       The bus on which to register the key store listener.
 * @param listener  The key store listener to set.
 *
 * @return
 *      - #ER_OK if the key store listener was set
 *      - #ER_BUS_LISTENER_ALREADY_SET if a listener has been set by this function or because
 *         EnablePeerSecurity has been called.
 */
extern AJ_API QStatus alljoyn_busattachment_registerkeystorelistener(alljoyn_busattachment bus, alljoyn_keystorelistener listener);

/**
 * Reloads the key store for this bus attachment. This function would normally only be called in
 * the case where a single key store is shared between multiple bus attachments, possibly by different
 * applications. It is up to the applications to coordinate how and when the shared key store is
 * modified.
 *
 * @param bus The bus on which to reload the key store.
 *
 * @return - ER_OK if the key store was succesfully reloaded
 *         - An error status indicating that the key store reload failed.
 */
extern AJ_API QStatus alljoyn_busattachment_reloadkeystore(alljoyn_busattachment bus);

/**
 * Clears all stored keys from the key store. All store keys and authentication information is
 * deleted and cannot be recovered. Any passwords or other credentials will need to be reentered
 * when establishing secure peer connections.
 *
 * @param bus The bus on which to clear the key store.
 */
extern AJ_API void alljoyn_busattachment_clearkeystore(alljoyn_busattachment bus);

/**
 * Clear the keys associated with aa specific remote peer as identified by its peer GUID. The
 * peer GUID associated with a bus name can be obtained by calling GetPeerGUID().
 *
 * @param bus   The bus from which to clear specific keys.
 * @param guid  The guid of a remote authenticated peer.
 *
 * @return  - ER_OK if the keys were cleared
 *          - ER_UNKNOWN_GUID if there is no peer with the specified GUID
 *          - Other errors
 */
extern AJ_API QStatus alljoyn_busattachment_clearkeys(alljoyn_busattachment bus, const char* guid);

/**
 * Set the expiration time on keys associated with a specific remote peer as identified by its
 * peer GUID. The peer GUID associated with a bus name can be obtained by calling GetPeerGUID().
 * If the timeout is 0 this is equivalent to calling ClearKeys().
 *
 * @param bus      The bus on which to set a key expiration.
 * @param guid     The GUID of a remote authenticated peer.
 * @param timeout  The time in seconds relative to the current time to expire the keys.
 *
 * @return  - ER_OK if the expiration time was succesfully set.
 *          - ER_UNKNOWN_GUID if there is no authenticated peer with the specified GUID
 *          - Other errors
 */
extern AJ_API QStatus alljoyn_busattachment_setkeyexpiration(alljoyn_busattachment bus, const char* guid, uint32_t timeout);

/**
 * Get the expiration time on keys associated with a specific authenticated remote peer as
 * identified by its peer GUID. The peer GUID associated with a bus name can be obtained by
 * calling GetPeerGUID().
 *
 * @param bus      The bus to query.
 * @param guid     The GUID of a remote authenticated peer.
 * @param timeout  The time in seconds relative to the current time when the keys will expire.
 *
 * @return  - ER_OK if the expiration time was succesfully set.
 *          - ER_UNKNOWN_GUID if there is no authenticated peer with the specified GUID
 *          - Other errors
 */
extern AJ_API QStatus alljoyn_busattachment_getkeyexpiration(alljoyn_busattachment bus, const char* guid, uint32_t* timeout);

/**
 * Adds a logon entry string for the requested authentication mechanism to the key store. This
 * allows an authenticating server to generate offline authentication credentials for securely
 * logging on a remote peer using a user-name and password credentials pair. This only applies
 * to authentication mechanisms that support a user name + password logon functionality.
 *
 * @param bus           The bus on which to add a logon entry.
 * @param authMechanism The authentication mechanism.
 * @param userName      The user name to use for generating the logon entry.
 * @param password      The password to use for generating the logon entry. If the password is
 *                      NULL the logon entry is deleted from the key store.
 *
 * @return
 *      - #ER_OK if the logon entry was generated.
 *      - #ER_BUS_INVALID_AUTH_MECHANISM if the authentication mechanism does not support
 *                                       logon functionality.
 *      - #ER_BAD_ARG_2 indicates a null string was used as the user name.
 *      - #ER_BAD_ARG_3 indicates a null string was used as the password.
 *      - Other error status codes indicating a failure
 */
extern AJ_API QStatus alljoyn_busattachment_addlogonentry(alljoyn_busattachment bus, const char* authMechanism,
                                                          const char* userName, const char* password);

/**
 * Add a DBus match rule.
 * This method is a shortcut/helper that issues an org.freedesktop.DBus.AddMatch method call to the local daemon.
 *
 * @parma[in]  bus   The bus on which to add the match rule.
 * @param[in]  rule  Match rule to be added (see DBus specification for format of this string).
 *
 * @return
 *      - #ER_OK if the AddMatch request was successful.
 *      - #ER_BUS_NOT_CONNECTED if a connection has not been made with a local bus.
 *      - Other error status codes indicating a failure.
 */
extern AJ_API QStatus alljoyn_busattachment_addmatch(alljoyn_busattachment bus, const char* rule);

/**
 * Remove a DBus match rule.
 * This method is a shortcut/helper that issues an org.freedesktop.DBus.RemoveMatch method call to the local daemon.
 *
 * @parma[in]  bus   The bus from which to remove the match rule.
 * @param[in]  rule  Match rule to be removed (see DBus specification for format of this string).
 *
 * @return
 *      - #ER_OK if the RemoveMatch request was successful.
 *      - #ER_BUS_NOT_CONNECTED if a connection has not been made with a local bus.
 *      - Other error status codes indicating a failure.
 */
extern AJ_API QStatus alljoyn_busattachment_removematch(alljoyn_busattachment bus, const char* rule);

/**
 * Set the SessionListener for an existing sessionId.
 * Calling this method will override the listener set by a previoius call to SetSessionListener or any
 * listener specified in JoinSession.
 *
 * @param bus          The bus on which to assign the SessionListener.
 * @param sessionId    The session id of an existing session.
 * @param listener     The SessionListener to associate with the session. May be NULL to clear previous listener.
 * @return  ER_OK if successful.
 */
extern AJ_API QStatus alljoyn_busattachment_setsessionlistener(alljoyn_busattachment bus, alljoyn_sessionid sessionId,
                                                               alljoyn_sessionlistener listener);

/**
 * Leave an existing session.
 * This method is a shortcut/helper that issues an org.alljoyn.Bus.LeaveSession method call to the local daemon
 * and interprets the response.
 *
 * @param[in]  bus           The bus on which to leave the session.
 * @param[in]  sessionId     Session id.
 *
 * @return
 *      - #ER_OK iff daemon response was received and the leave operation was successfully completed.
 *      - #ER_BUS_NOT_CONNECTED if a connection has not been made with a local bus.
 *      - Other error status codes indicating a failure.
 */
extern AJ_API QStatus alljoyn_busattachment_leavesession(alljoyn_busattachment bus, alljoyn_sessionid sessionId);

/**
 * Set the link timeout for a session.
 *
 * Link timeout is the maximum number of seconds that an unresponsive daemon-to-daemon connection
 * will be monitored before declaring the session lost (via SessionLost callback). Link timeout
 * defaults to 0 which indicates that AllJoyn link monitoring is disabled.
 *
 * Each transport type defines a lower bound on link timeout to avoid defeating transport
 * specific power management algorithms.
 *
 * @param bus           The bus containing the link to modify.
 * @param sessionid     Id of session whose link timeout will be modified.
 * @param linkTimeout   [IN/OUT] Max number of seconds that a link can be unresponsive before being
 *                      declared lost. 0 indicates that AllJoyn link monitoring will be disabled. On
 *                      return, this value will be the resulting (possibly upward) adjusted linkTimeout
 *                      value that acceptable to the underlying transport.
 *
 * @return
 *      - #ER_OK if successful
 *      - #ER_ALLJOYN_SETLINKTIMEOUT_REPLY_NOT_SUPPORTED if local daemon does not support SetLinkTimeout
 *      - #ER_ALLJOYN_SETLINKTIMEOUT_REPLY_NO_DEST_SUPPORT if SetLinkTimeout not supported by destination
 *      - #ER_BUS_NO_SESSION if the Session id is not valid
 *      - #ER_ALLJOYN_SETLINKTIMEOUT_REPLY_FAILED if SetLinkTimeout failed
 *      - #ER_BUS_NOT_CONNECTED if the BusAttachment is not connected to the daemon
 */
extern AJ_API QStatus alljoyn_busattachment_setlinktimeout(alljoyn_busattachment bus, alljoyn_sessionid sessionid, uint32_t* linkTimeout);

/**
 * Determine whether a given well-known name exists on the bus.
 * This method is a shortcut/helper that issues an org.freedesktop.DBus.NameHasOwner method call to the daemon
 * and interprets the response.
 *
 * @param[in]  bus        The bus to query.
 * @param[in]  name       The well known name that the caller is inquiring about.
 * @param[out] hasOwner   If return is ER_OK, indicates whether name exists on the bus.
 *                        If return is not ER_OK, param is not modified.
 * @return
 *      - #ER_OK if name ownership was able to be determined.
 *      - An error status otherwise
 */
extern AJ_API QStatus alljoyn_busattachment_namehasowner(alljoyn_busattachment bus, const char* name, QCC_BOOL* hasOwner);

/**
 * Get the peer GUID for this peer of the local peer or an authenticated remote peer. The bus
 * names of a remote peer can change over time, specifically the unique name is different each
 * time the peer connects to the bus and a peer may use different well-known-names at different
 * times. The peer GUID is the only persistent identity for a peer. Peer GUIDs are used by the
 * authentication mechanisms to uniquely and identify a remote application instance. The peer
 * GUID for a remote peer is only available if the remote peer has been authenticated.
 *
 * @param bus    The bus on which to get the peer GUID.
 * @param name   Name of a remote peer or NULL to get the local (this application's) peer GUID.
 * @param guid   Returns the guid for the local or remote peer depending on the value of name, or NULL to get size.
 * @param guidSz Size of the provided guid buffer, upon exit size of guid string.
 *
 * @return
 *      - #ER_OK if the requested GUID was obtained.
 *      - An error status otherwise.
 */
extern AJ_API QStatus alljoyn_busattachment_getpeerguid(alljoyn_busattachment bus, const char* name, char* guid, size_t* guidSz);

/**
 * This sets the debug level of the local AllJoyn daemon if that daemon
 * was built in debug mode.
 *
 * The debug level can be set for individual subsystems or for "ALL"
 * subsystems.  Common subsystems are "ALLJOYN" for core AllJoyn code,
 * "ALLJOYN_OBJ" for the sessions management code, "ALLJOYN_BT" for the
 * Bluetooth subsystem, "ALLJOYN_BTC" for the Bluetooth topology manager,
 * and "ALLJOYN_NS" for the TCP name services.  Debug levels for specific
 * subsystems override the setting for "ALL" subsystems.  For example if
 * "ALL" is set to 7, but "ALLJOYN_OBJ" is set to 1, then detailed debug
 * output will be generated for all subsystems expcept for "ALLJOYN_OBJ"
 * which will only generate high level debug output.  "ALL" defaults to 0
 * which is off, or no debug output.
 *
 * The debug output levels are actually a bit field that controls what
 * output is generated.  Those bit fields are described below:
 *
 *     - 0x1: High level debug prints (these debug printfs are not common)
 *     - 0x2: Normal debug prints (these debug printfs are common)
 *     - 0x4: Function call tracing (these debug printfs are used
 *            sporadically)
 *     - 0x8: Data dump (really only used in the "SOCKET" module - can
 *            generate a *lot* of output)
 *
 * Typically, when enabling debug for a subsystem, the level would be set
 * to 7 which enables High level debug, normal debug, and function call
 * tracing.  Setting the level 0, forces debug output to be off for the
 * specified subsystem.
 *
 * @param bus       bus on which to set debugging.
 * @param module    name of the module to generate debug output
 * @param level     debug level to set for the module
 *
 * @return
 *     - #ER_OK if debug request was successfully sent to the AllJoyn
 *       daemon.
 *     - #ER_BUS_NO_SUCH_OBJECT if daemon was not built in debug mode.
 */
extern AJ_API QStatus alljoyn_busattachment_setdaemondebug(alljoyn_busattachment bus, const char* module, uint32_t level);

/**
 * Returns the current non-absolute real-time clock used internally by AllJoyn. This value can be
 * compared with the timestamps on messages to calculate the time since a timestamped message
 * was sent.
 *
 * @return  The current timestamp in milliseconds.
 */
extern AJ_API uint32_t alljoyn_busattachment_gettimestamp();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif