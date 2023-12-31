/*----------------------------------------------------------------------------*
 *  Copyright (c) 2001        Southeastern Universities Research Association, *
 *                            Thomas Jefferson National Accelerator Facility  *
 *                                                                            *
 *    This software was developed under a United States Government license    *
 *    described in the NOTICE file included as part of this distribution.     *
 *                                                                            *
 *    Author:  Carl Timmer                                                    *
 *             timmer@jlab.org                   Jefferson Lab, MS-12H        *
 *             Phone: (757) 269-5130             12000 Jefferson Ave.         *
 *             Fax:   (757) 269-5800             Newport News, VA 23606       *
 *                                                                            *
 *----------------------------------------------------------------------------*/

package org.jlab.coda.et;

import java.lang.*;
import java.io.*;
import java.net.*;
import java.util.*;

/**
 * This class opens (finds and connects to) an ET system. The use of this class
 * is hidden from the user. There should be no reason to use it.
 *
 * @author Carl Timmer
 */

public class SystemOpen {

  /** Object specifying how to open an ET system. */
  SystemOpenConfig config;
  /** TCP socket connection established with an ET system's server. */
  Socket   sock;
  /** Name of the host the ET system resides on. */
  String   host;
  /** Port number of the ET system's tcp server. */
  int      tcpPort;
  /** Debug level. Set by {@link SystemOpen#setDebug}. */
  int      debug;

  // properties of opened ET system

  /** Endian value of the opened ET system. */
  int  endian;
  /** Total number of events of the opened ET system. */
  int  numEvents;
  /** Event size in bytes of the opened ET system. */
  long  eventSize;
  /** Major version number of the opened ET system. */
  int  version;
  /** Number of select integers in the opened ET system. */
  int  stationSelectInts;
  /** Language used to implement the opened ET system. The possible values are
   *  {@link Constants#langJava} for Java, {@link Constants#langCpp} for C++,
   *  and {@link Constants#langC} for C. */
  int  language;
  /** True if ET system is 64 bit, else false. */
  boolean bit64;

  // set of all ET systems that respond - host & port

  /** In case of multiple responding ET systems, a list of their hosts. */
  ArrayList<String> respondingHosts;
  /** In case of multiple responding ET systems, a list of their ports. */
  ArrayList<Integer> respondingPorts;

  // status indicators
  private boolean connected;
  private final boolean  foundServer=true, cannotFindServer=false;
  private final boolean  gotMatch=true,    noMatch=false;


  /** Create a SystemOpen object.
   *  @param _config SystemOpenConfig object  */
  public SystemOpen (SystemOpenConfig _config) {
    config = new SystemOpenConfig(_config);
    debug = Constants.debugError;
    respondingHosts = new ArrayList<String>(20);
    respondingPorts = new ArrayList<Integer>(20);
  }

  // public sets
  
  /** Sets the debug output level. Must be either {@link Constants#debugNone},
   *  {@link Constants#debugSevere}, {@link Constants#debugError},
   *  {@link Constants#debugWarn}, or {@link Constants#debugInfo}.
   *  @param _debug debug level
   *  @exception org.jlab.coda.et.EtException
   *     if bad argument value
   */
  public void setDebug(int _debug) throws EtException {
    if ((_debug != Constants.debugNone)   &&
        (_debug != Constants.debugSevere) &&
        (_debug != Constants.debugError)  &&
        (_debug != Constants.debugWarn)   &&
        (_debug != Constants.debugInfo))    {
      throw new EtException("bad debug argument");
    }
    debug = _debug;
  }
  
  
  // public gets

  /** Gets the total number of events of the opened ET system.
   *  @return total number of events */
  public int getNumEvents() {return numEvents;}
  /** Gets the size of the normal events in bytes of the opened ET system.
   *  @return size of normal events in bytes */
  public long getEventSize() {return eventSize;}
  /** Gets the tcp server port number of the opened ET system.
   *  @return tcp server port number */
  public int getPort()      {return tcpPort;}
  /** Gets the host name the opened ET system is running on.
   *  @return host name */
  public String getHost()   {return host;}
  /** Gets the name of the ET system (file).
   *  @return ET system name */
  public String getName()   {return config.name;}
  /** Gets the endian value of the opened ET system.
   *  @return endian value */
  public int getEndian() {return endian;}
  /** Gets the major version number of the opened ET system.
   *  @return major ET version number */
  public int getVersion() {return version;}
  /** Gets the language used to implement the opened ET system..
   *  @return language */
  public int getLanguage() {return language;}
  /** Gets the number of station select integers of the opened ET system..
   *  @return number of select integers */
  public int getSelectInts() {return stationSelectInts;}
  /** Gets the socket connecting this object to the ET system..
   *  @return socket */
  public Socket getSocket() {return sock;}
   
  /** Gets the debug output level.
   *  @return debug output level */
  public int getDebug() {return debug;}
  /** Gets a copy of the SystemOpenConfig configuration object.
   *  @return configuration object */
  public SystemOpenConfig getConfig()   {return new SystemOpenConfig(config);}
  /** Gets whether the ET system is connected (opened) or not.
   *  @return status of connection to ET system */
  synchronized public boolean isConnected() {return connected;}

  // The next two methods are really only useful when
  // the EtTooManyException is thrown from "connect"
  // or findServerPort.

  /** Gets all host names when multiple ET systems respond.
   *  @return all host names from responding ET systems */
  public String[] getAllHosts() {
    int size = respondingHosts.size();
    if (size == 0) {
      if (host == null) {
        return null;
      }
      return new String[] {host};
    }
    String[] hosts = new String[size];
    for (int i=0; i < size; i++) {
      hosts[i] = (String) respondingHosts.get(i);
    }
    return hosts;
  }

  /** Gets all port numbers when multiple ET systems respond.
   *  @return all port numbers from responding ET systems */
  public int[] getAllPorts() {
    int size = respondingPorts.size();
    if (size == 0) {
      if (tcpPort == 0) {
        return null;
      }
      return new int[] {tcpPort};
    }
    int[] ports = new int[size];
    for (int i=0; i < size; i++) {
      ports[i] = ((Integer) respondingPorts.get(i)).intValue();
    }
    return ports;
  }


  /**
   * Finds the ET system's tcp server port number.
   *
   * @exception java.io.IOException
   *     if problems with network comunications
   * @exception java.net.UnknownHostException
   *     if the host address(es) is(are) unknown
   * @exception org.jlab.coda.et.EtTooManyException
   *     if there were more than one valid response when policy is set to
   *     {@link Constants#policyError} and we are looking either
   *     remotely or anywhere for the ET system.
   */
  private boolean findServerPort() throws IOException, UnknownHostException, EtTooManyException {
    boolean match = noMatch;
    int     status, totalPacketsSent = 0, sendPacketLimit = 4;
    int     timeOuts[] = {50, 2000, 4000, 7000};
    int     waitTime, socketTimeOut = 20000; // socketTimeOut > sum of timeOuts
    String  specifiedHost = null;
    
    // clear out any previously stored objects
    respondingHosts.clear();
    respondingPorts.clear();

    // Put outgoing packet info into a byte array to send to ET systems
    ByteArrayOutputStream  baos = new ByteArrayOutputStream(110);
    DataOutputStream        dos = new DataOutputStream(baos);
    // write ET version
    dos.writeInt(Constants.version);
    // write string length of ET name
    dos.writeInt(config.name.length() + 1);
    // write ET name
    try {
      dos.write(config.name.getBytes("ASCII"));
      dos.writeByte(0);
    }
    catch (UnsupportedEncodingException ex) {/* will never happen */}
    dos.flush();
    
    // construct byte array to send over a socket
    final byte sbuffer[] = baos.toByteArray();
    dos.close();
    baos.close();

    // We may need to send packets over many different sockets
    // as there may be broadcasting on multiple subnets as well
    // as multicasts on several addresses. Keep track of these
    // sockets, addresses, & packets with this class:
    class send {
      int             port;
      String          address;
      InetAddress     addr;
      MulticastSocket socket;
      DatagramPacket  packet;

      send (String _address, MulticastSocket _socket, int _port) throws UnknownHostException {
        port    = _port;
        address = _address;
	    socket  = _socket;
	    addr    = InetAddress.getByName(address);  //UnknownHostEx
        packet  = new DatagramPacket(sbuffer, sbuffer.length, addr, port);
      }
    }

    int index=0;
    int numBroadcastAddrs = config.isBroadcasting() ? 1 : 0;
    int numMulticastAddrs = config.getNumMulticastAddrs();
    int numAddrs = numBroadcastAddrs + numMulticastAddrs + 1;
    send[] sendIt = new send[numAddrs];

    // find local host
    String localHost = null;
    try {localHost = InetAddress.getLocalHost().getHostName();}
    catch (UnknownHostException ex) {}

    // If the host is not remote or anywhere out there. If it's
    // local or we know its name, send a UDP packet to it alone.
    if ((!config.host.equals(Constants.hostRemote)) &&
        (!config.host.equals(Constants.hostAnywhere)))  {

      // We can use multicast socket for regular UDP - it works
      MulticastSocket socket = new MulticastSocket();	//IOEx
      // Socket will unblock after timeout,
      // letting reply collecting thread quit
      try {socket.setSoTimeout(socketTimeOut);}
      catch (SocketException ex) {}

      // if it's local ...
      if ((config.host.equals(Constants.hostLocal)) ||
          (config.host.equals("localhost")))  {
	    specifiedHost = localHost;
      // else if we know host's name ...
      } else {
	    specifiedHost = config.host;
      }
      sendIt[index++] = new send(specifiedHost, socket, config.udpPort);
      numAddrs = 1;

        // setup multicast sockets & packets next
        if ((config.contactMethod == Constants.multicast) ||
                (config.contactMethod == Constants.broadAndMulticast)) {

            for (String addr : config.multicastAddrs) {
                MulticastSocket sock = new MulticastSocket();    //IOEx
                try {
                    sock.setSoTimeout(socketTimeOut);
                }
                catch (SocketException ex) {
                }

                if (config.ttl != 1) {
                    sock.setTimeToLive(config.ttl);        //IOEx
                }

                sendIt[index++] = new send(addr, sock, config.multicastPort);
                if (debug >= Constants.debugInfo) {
                    System.out.println("findServerPort: multicasting to " + addr + " on port " + config.multicastPort);
                }
            }
        }
        else {
            numMulticastAddrs = 0;
        }
        numAddrs += numMulticastAddrs;

        if (debug >= Constants.debugInfo) {
            System.out.println("findServerPort: send to local or specified host " + specifiedHost +
            " on port " + config.udpPort);
        }
    }
    else {

        // setup broadcast sockets & packets first
        if ((config.contactMethod == Constants.broadcast) ||
                (config.contactMethod == Constants.broadAndMulticast)) {

            // We can use multicast socket for broadcasting - it works
            MulticastSocket socket = new MulticastSocket();    //IOEx
            // Socket will unblock after timeout,
            // letting reply collecting thread quit
            try {
                socket.setSoTimeout(socketTimeOut);
                socket.setBroadcast(true);
            }
            catch (SocketException ex) {
            }

            sendIt[index++] = new send(config.broadcastIP, socket, config.udpPort);
            if (debug >= Constants.debugInfo) {
                System.out.println("findServerPort: broadcasting to " + config.broadcastIP +
                " on port " + config.udpPort);
            }
            numBroadcastAddrs = 1;
        }
        else {
            numBroadcastAddrs = 0;
        }

        // setup multicast sockets & packets next
        if ((config.contactMethod == Constants.multicast) ||
                (config.contactMethod == Constants.broadAndMulticast)) {

            for (String addr : config.multicastAddrs) {
                MulticastSocket socket = new MulticastSocket();    //IOEx
                try {
                    socket.setSoTimeout(socketTimeOut);
                }
                catch (SocketException ex) {
                }

                if (config.ttl != 1) {
                    socket.setTimeToLive(config.ttl);        //IOEx
                }

                sendIt[index++] = new send(addr, socket, config.multicastPort);
                if (debug >= Constants.debugInfo) {
                    System.out.println("findServerPort: multicasting to " + addr + " on port " + config.multicastPort);
                }
            }
        }
        else {
            numMulticastAddrs = 0;
        }
        numAddrs = numBroadcastAddrs + numMulticastAddrs;
    }

      /** Class to help receive a packet on a socket. */
      class get {
          // data size = 4*4 + 2 + Constants.ipAddrStrLen +
          //             2*Constants.maxHostNameLen(); = 546 bytes
          // but give us a bit of extra room with 1k bytes
          byte[] buffer = new byte[1000];
          DatagramReceive thread;
          DatagramPacket packet;
          MulticastSocket socket;

          get(MulticastSocket sock) {
              packet = new DatagramPacket(buffer, buffer.length);
              socket = sock;
          }

          // start up thread to receive single udp packet on single socket
          void start() {
              thread = new DatagramReceive(packet, socket);
              thread.start();
          }
      }

      // store things here
      get[] receiveIt = new get[numAddrs];

      // start reply collecting threads
      for (int i = 0; i < numAddrs; i++) {
          receiveIt[i] = new get(sendIt[i].socket);
          // start single thread
          if (debug >= Constants.debugInfo) {
              System.out.println("findServerPort: starting thread to socket " + sendIt[i].socket);
          }
          receiveIt[i].start();
      }

      Thread.yield();

      send:
      // set a limit on the total # of packet groups sent out to find a server
      while (totalPacketsSent < sendPacketLimit) {
          // send packets out on all sockets
          for (int i = 0; i < numAddrs; i++) {
              sendIt[i].socket.send(sendIt[i].packet); //IOException
          }
          // set time to wait for reply (gets longer with each round)
          waitTime = timeOuts[totalPacketsSent++];

          get:
          while (true) {
              if (debug >= Constants.debugInfo) {
                  System.out.println("findServerPort: wait for " + waitTime + " milliseconds");
              }
              // wait for replies
              try {
                  Thread.sleep(waitTime);
              }
              catch (InterruptedException ix) {
              }

              // check for replies on all sockets
              for (int i = 0; i < numAddrs; i++) {
                  status = receiveIt[i].thread.waitForReply(10);
                  if (debug >= Constants.debugInfo) {
                      System.out.println("findServerPort: receive on socket " + receiveIt[i].socket +
                              ", status = " + status);
                  }

                  // if error or timeout ...
                  if ((status == DatagramReceive.error) || (status == DatagramReceive.timedOut)) {
                      // continue;
                  }

                  // else if got packet ...
                  else if (status == DatagramReceive.receivedPacket) {
                      // Analyze packet to see it matches the ET system we were
                      // looking for; if not, try to get another packet. If it
                      // is a match, store it in a HashMap (responders).
                      if (replyMatch(receiveIt[i].packet)) { // IOEx, UnknownHostEx
                          if (debug >= Constants.debugInfo) {
                              System.out.println("findServerPort: found match");
                          }
                          match = gotMatch;
                      }
                      else {
                          if (debug >= Constants.debugInfo) {
                              System.out.println("findServerPort: no match");
                          }
                      }
                      // See if there are other packets cued up,
                      // but don't wait too long. The thread we
                      // started is ended so start another up again.
                      waitTime = 50;
                      receiveIt[i].start();
                      Thread.yield();

                      continue get;
                  }
              }

              // if we don't have a match, try again
              if (!match) {
                  // If max # of packets not yet sent, send another
                  // batch and try again with a longer wait
                  if (totalPacketsSent < sendPacketLimit) {
                      if (debug >= Constants.debugInfo) {
                          System.out.println("findServerPort: timedout, try again with longer wait");
                      }
                      continue send;
                  }
              }

              break send;

          } // while (true)
      } // while (totalPacketsSent < sendPacketLimit)


      if (match) {
          // If the host is not remote or anywhere out there (i.e. we
          // know its name).
          if ((!config.host.equals(Constants.hostRemote)) &&
              (!config.host.equals(Constants.hostAnywhere))) {

              // if we have more than one responding ET system ...
              if (respondingHosts.size() > 1) {
                  // pick first ET system that matches the specified host's name
                  for (String listedHost : respondingHosts) {
                      if (specifiedHost.equals(listedHost)) {
                          host = listedHost;
                          tcpPort = ((Integer) respondingPorts.get
                                  (respondingHosts.indexOf(listedHost))).intValue();
                          return foundServer;
                      }
                  }
              }

          }
          // if we're looking remotely or anywhere
          else {
              // if we have more than one responding ET system
              if (respondingHosts.size() > 1) {
                  // if picking first responding ET system ...
                  if (config.responsePolicy == Constants.policyFirst) {
                      host = (String) respondingHosts.get(0);
                      tcpPort = ((Integer) respondingPorts.get(0)).intValue();
                  }
                  // else if picking local system first ...
                  else if (config.responsePolicy == Constants.policyLocal) {
                      // compare local host to responding hosts
                      boolean foundLocalHost = false;
                      for (String listedHost : respondingHosts) {
                          if (localHost.equals(listedHost)) {
                              host = listedHost;
                              tcpPort = ((Integer) respondingPorts.get
                                      (respondingHosts.indexOf(listedHost))).intValue();
                              foundLocalHost = true;
                          }
                      }
                      // if no local host found, pick first responder
                      if (!foundLocalHost) {
                          host = (String) respondingHosts.get(0);
                          tcpPort = ((Integer) respondingPorts.get(0)).intValue();
                      }
                  }
                  // else if policy.Error
                  else {
                      throw new EtTooManyException("too many responding ET systems");
                  }
              }
          }
          return foundServer;
      }
      if (debug >= Constants.debugInfo) {
          System.out.println("findServerPort: cannot find server, quitting");
      }
      host = null;
      tcpPort = 0;
      return cannotFindServer;
  }


  /**
   * Analyze a received UDP packet & see if it matches the ET system we're
   * looking for.
   *
   * @param packet responding UDP packet
   * @exception java.io.IOException
   *     if problems with network comunications
   * @exception java.net.UnknownHostException
   *     if the replied host address(es) is(are) unknown
   */
  private boolean replyMatch(DatagramPacket packet)
			throws IOException, UnknownHostException {
      int port, length;
      byte buf[];
      ByteArrayInputStream bais = new ByteArrayInputStream(packet.getData());
      DataInputStream dis = new DataInputStream(bais);

      // read ET version #
      dis.readInt();         //IOEx
      // server port #
      port = dis.readInt();

      // read length of fully qualified hostname of responding interface
      length = dis.readInt();
      // read fully qualified ET server host name (minus ending null)
      buf = new byte[length];
      dis.readFully(buf, 0, length);
      String repliedHostName = null;
      try {repliedHostName = new String(buf, 0, length - 1, "ASCII");}
      catch (UnsupportedEncodingException e) {/*never happen*/}

      // read length of IP address (dotted-decimal) of responding interface
      length = dis.readInt();
      // read IP address
      buf = new byte[length];
      dis.readFully(buf, 0, length);
      String repliedIpAddress = null;
      try {repliedIpAddress = new String(buf, 0, length - 1, "ASCII");}
      catch (UnsupportedEncodingException e) {}

      // Read length of fully qualified hostname from "uname".
      // Used as identifier of this host no matter which interface used.
      length = dis.readInt();
      // read uname
      buf = new byte[length];
      dis.readFully(buf, 0, length);
      String repliedUname = null;
      try {repliedUname = new String(buf, 0, length - 1, "ASCII");}
      catch (UnsupportedEncodingException e) {}


      if (debug >= Constants.debugInfo) {
          System.out.println("replyMatch: port = " + port +
                  ", server = " + repliedHostName +
                  ", IP addr = " + repliedIpAddress +
                  ", uname = " + repliedUname);
      }

      dis.close();
      bais.close();

      // set ip address values for replied & local host
      InetAddress repliedHost = InetAddress.getByName(repliedHostName);   //UnknownHostEx
      InetAddress localHost   = InetAddress.getLocalHost();               //UnknownHostEx

      // if we're looking for a host anywhere
      if (config.host.equals(Constants.hostAnywhere)) {
          if (debug >= Constants.debugInfo) {
              System.out.println("replyMatch: .anywhere");
          }

          // Store host & port in lists in case there are several systems
          // that respond and user must chose which one he wants.
          respondingHosts.add(repliedHostName);
          respondingPorts.add(new Integer(port));

          // store info here in case only 1 response
          host = repliedHostName;
          tcpPort = port;
          return gotMatch;
      }
      // else if we're looking for a remote host
      else if (config.host.equals(Constants.hostRemote)) {
          if (debug >= Constants.debugInfo) {
              System.out.println("replyMatch: .remote");
          }
          if (!localHost.equals(repliedHost)) {
              // Store host & port in lists in case there are several systems
              // that respond and user must chose which one he wants
              respondingHosts.add(repliedHostName);
              respondingPorts.add(new Integer(port));

              // store info here in case only 1 response
              host = repliedHostName;
              tcpPort = port;
              return gotMatch;
          }
      }
      // else if we're looking for a local host
      else if ((config.host.equals(Constants.hostLocal)) ||
               (config.host.equals("localhost"))) {
          if (debug >= Constants.debugInfo) {
              System.out.println("replyMatch: .local");
          }
          if (localHost.equals(repliedHost)) {
              // Store values. In this case no other match will be found.
              host = repliedHostName;
              tcpPort = port;
              return gotMatch;
          }
      }
      // else a specific host name has been specified
      else {
          if (debug >= Constants.debugInfo) {
              System.out.println("replyMatch: <name>");
          }
          // "config.host" is the host name we're looking for
          InetAddress etHost = InetAddress.getByName(config.host);    //UnknownHostEx
          if (etHost.equals(repliedHost)) {
              // Store values. In this case no other match will be found.
              host = repliedHostName;
              tcpPort = port;
              return gotMatch;
          }
      }


      return noMatch;
  }


  /**
   * Open an ET system.
   *
   * @exception java.io.IOException
   *     if problems with network comunications
   * @exception org.jlab.coda.et.EtException
   *     if the responing ET system has the wrong name, runs a different version
   *     of ET, or has a different value for {@link Constants#stationSelectInts}
   */
  private void open() throws IOException, EtException {
    DataInputStream  dis = new DataInputStream(sock.getInputStream());
    DataOutputStream dos = new DataOutputStream(sock.getOutputStream());

    // write our endian, length of ET filename, and ET filename
    dos.writeInt(Constants.endianBig);
    dos.writeInt(config.name.length() + 1);
    dos.writeInt(0);    // 1 means 64 bit, 0 means 32 bit (all java is 32 bit)
    dos.writeLong(0L);	// write one 64 bit long instead of 2, 32 bit ints since = 0 anyway
    try {
      dos.write(config.name.getBytes("ASCII"));
      dos.writeByte(0);
    }
    catch (UnsupportedEncodingException ex) {/* will never happen */}
    dos.flush();

    // read what ET's tcp server sends back
    if (dis.readInt() != Constants.ok) {
      throw new EtException("found the wrong ET system");
    }
    endian            = dis.readInt();
    numEvents         = dis.readInt();
    eventSize         = dis.readLong();
    version           = dis.readInt();
    stationSelectInts = dis.readInt();
    language          = dis.readInt();
    bit64             = dis.readInt() > 0;
    dis.skipBytes(4);

    // check to see if connecting to same version ET software
    if (version != Constants.version) {
      disconnect();
      throw new EtException("may not open wrong version ET system");
    }
    // double check to see if # of select ints are the same
    if (stationSelectInts != Constants.stationSelectInts) {
      disconnect();
      throw new EtException("may not open ET system with different # of select integers");
    }
    
    connected = true;
    
    if (debug >= Constants.debugInfo) {
      System.out.println("open: endian = " + endian +
    		 ", nevents = " + numEvents +
    		 ", event size = " + eventSize +
		     ", version = " + version +
		     ",\n      selectInts = " + stationSelectInts +
		     ", language = " + language);
    }
    return;
  }


  /**
   * Sets up a TCP socket connection to an ET system.
   *
   * @exception java.io.IOException
   *     if problems with network comunications
   * @exception java.net.UnknownHostException
   *     if the host address(es) is(are) unknown
   * @exception org.jlab.coda.et.EtException
   *     if the responding ET system has the wrong name, runs a different
   *     version of ET, or has a different value for
   *     {@link Constants#stationSelectInts}
   * @exception org.jlab.coda.et.EtTooManyException
   *     if there were more than one valid response when policy is set to
   *     {@link Constants#policyError} and we are looking either
   *     remotely or anywhere for the ET system.
   */
  synchronized public void connect() throws IOException, UnknownHostException,
                        EtException, EtTooManyException {

      if (config.contactMethod == Constants.direct) {
          // if making direct connection, we have host & port
          if (debug >= Constants.debugInfo) {
              System.out.println("connect: make a direct connection");
          }
          tcpPort = config.tcpPort;

          // if "local" specified, find actual hostname
          if (config.host.equals(Constants.hostLocal)) {
              host = InetAddress.getLocalHost().getHostName();
          }
          else {
              // We prefer a fully qualifed host name. If there are no "."'s
              // in it, try getHostName even though that is not guaranteed
              // to return a fully qualified name.
              if (config.host.indexOf(".") < 0) {
                  if (debug >= Constants.debugInfo) {
                      System.out.println("connect: try to make " + config.host + " a fully qualified name");
                  }
                  host = InetAddress.getByName(config.host).getHostName();
              }
              else {
                  host = config.host;
              }
          }
      }
      else {
          if (debug >= Constants.debugInfo) {
              System.out.println("connect: try to find server port");
          }
          // send a UDP broad or multicast packet to find ET TCP server & port
          if (!findServerPort()) {    // IOEx, UnknownHostEx, EtTooMany
              throw new EtException("Cannot find ET system");
          }
      }

      // Open our connection to an ET system TCP Server
      sock = new Socket(host, tcpPort);        // IOEx
      try {
          // Set NoDelay option for fast response
          sock.setTcpNoDelay(true);
          // Set reading timeout to 2 second so dead ET sys
          // can be found by reading on a socket.
          sock.setSoTimeout(2000);
          // Set KeepAlive so we can tell if ET system is dead
          sock.setKeepAlive(true);
          // set buffer size
          sock.setReceiveBufferSize(65535);
          sock.setSendBufferSize(65535);
      }
      catch (SocketException ex) {
      }

      // open the ET system
      if (debug >= Constants.debugInfo) {
          System.out.println("connect: try to open ET system");
      }
      open();    // IOEx, EtEx
  }


  synchronized public void disconnect() {
    connected = false;
    try {sock.close();}
    catch (IOException ioex) {}
  }
}



/**
 * This class is designed to receive UDP packets.
 *
 * @author Carl Timmer
 * @version 6.0
 */

class DatagramReceive extends Thread {

  /** UDP Packet in which to receive communication data. */
  DatagramPacket packet;
  /** UDP Socket over which to communicate. */
  DatagramSocket socket;

  // allowed states

  /** Status of timed out. */
  static final int timedOut = 0;
  /** Status of packet received. */
  static final int receivedPacket = 1;
  /** Status of error. */
  static final int error = -1;

  /** Current status. */
  volatile int status = timedOut;

  /**
   * Creates a DatagramReceive object.
   * @param recvPacket UDP packet in which to receive communication data.
   * @param recvSocket UDP Socket over which to communicate
   *
   */
  DatagramReceive(DatagramPacket recvPacket,
                         DatagramSocket recvSocket)
  {
    packet = recvPacket;
    socket = recvSocket;
  }

  // this needs to be synchronized so the "wait" will work

  /**
   * Waits for a UDP packet to be received.
   * @param time time to wait in milliseconds before timing out.
   */
  synchronized int waitForReply(int time)
  {
    // If the thread was started before we got a chance to wait for the
    // reply, check to see if a reply has already come.
    if (status != timedOut) {
      return status;
    }

    try {wait(time);}
    catch (InterruptedException intEx) {}
    return status;
  }


  // No need to synchronize run as it can only be called once
  // by this object. Furthermore, if it is synchronized, then
  // if no packet is received, it is blocked with the mutex.
  // That, in turn, does not let the wait statement return from
  // a timeout. Since run is the only method that changes "status",
  // status does not have to be mutex-protected.

  /**
   * Method to run thread to receive UDP packet and notify waiters.
   */
  public void run()
  {
    status = timedOut;
    try {
      socket.receive(packet);
      status = receivedPacket;
    }
    catch (IOException iox) {
      status = error;
      return;
    }
    synchronized (this) {
      notify();
    }
  }

}




