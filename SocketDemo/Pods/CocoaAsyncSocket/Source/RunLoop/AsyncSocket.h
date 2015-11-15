//
//  AsyncSocket.h
//  
//  This class is in the public domain.
//  Originally created by Dustin Voss on Wed Jan 29 2003.
//  Updated and maintained by Deusty Designs and the Mac development community.
//
//  http://code.google.com/p/cocoaasyncsocket/
//

#import <Foundation/Foundation.h>

@class AsyncSocket;
@class AsyncReadPacket;
@class AsyncWritePacket;

extern NSString *const AsyncSocketException;
extern NSString *const AsyncSocketErrorDomain;

typedef NS_ENUM(NSInteger, AsyncSocketError) {
	AsyncSocketCFSocketError = kCFSocketError,	// From CFSocketError enum.
	AsyncSocketNoError = 0,						// Never used.
	AsyncSocketCanceledError,					// onSocketWillConnect: returned NO.
	AsyncSocketConnectTimeoutError,
	AsyncSocketReadMaxedOutError,               // Reached set maxLength without completing
	AsyncSocketReadTimeoutError,
	AsyncSocketWriteTimeoutError
};

@protocol AsyncSocketDelegate
@optional

/**
 * In the event of an error, the socket is closed.
 * You may call "unreadData" during this call-back to get the last bit of data off the socket.
 * When connecting, this delegate method may be called
 * before"onSocket:didAcceptNewSocket:" or "onSocket:didConnectToHost:".
**/
//发 生错误，socket关闭，可以在call-back过程调用"unreadData"去取得socket的最后的数据字节，当连接的时候，该委托方法 在    onSocket:didAcceptNewSocket: 或者 onSocket:didConnectToHost： 之前调用
- (void)onSocket:(AsyncSocket *)sock willDisconnectWithError:(NSError *)err;

/**
 * Called when a socket disconnects with or without error.  If you want to release a socket after it disconnects,
 * do so here. It is not safe to do that during "onSocket:willDisconnectWithError:".
 * 
 * If you call the disconnect method, and the socket wasn't already disconnected,
 * this delegate method will be called before the disconnect method returns.
**/
//当socket由于或没有错误而断开连接，如果你想要在断开连接后release socket，在此方法工作，而在onSocket:willDisconnectWithError 释放则不安全
- (void)onSocketDidDisconnect:(AsyncSocket *)sock;

/**
 * Called when a socket accepts a connection.  Another socket is spawned to handle it. The new socket will have
 * the same delegate and will call "onSocket:didConnectToHost:port:".
**/
//当产生一个socket去处理连接时调用，此方法会返回 线程上的run-loop 的新的socket和其应处理的委托，如果省略，则使用[NSRunLoop cunrrentRunLoop]
- (void)onSocket:(AsyncSocket *)sock didAcceptNewSocket:(AsyncSocket *)newSocket;

/**
 * Called when a new socket is spawned to handle a connection.  This method should return the run-loop of the
 * thread on which the new socket and its delegate should operate. If omitted, [NSRunLoop currentRunLoop] is used.
**/
- (NSRunLoop *)onSocket:(AsyncSocket *)sock wantsRunLoopForNewSocket:(AsyncSocket *)newSocket;

/**
 * Called when a socket is about to connect. This method should return YES to continue, or NO to abort.
 * If aborted, will result in AsyncSocketCanceledError.
 * 
 * If the connectToHost:onPort:error: method was called, the delegate will be able to access and configure the
 * CFReadStream and CFWriteStream as desired prior to connection.
 *
 * If the connectToAddress:error: method was called, the delegate will be able to access and configure the
 * CFSocket and CFSocketNativeHandle (BSD socket) as desired prior to connection. You will be able to access and
 * configure the CFReadStream and CFWriteStream in the onSocket:didConnectToHost:port: method.
**/
- (BOOL)onSocketWillConnect:(AsyncSocket *)sock;

/**
 * Called when a socket connects and is ready for reading and writing.
 * The host parameter will be an IP address, not a DNS name.
**/
//当socket连接正准备读和写的时候调用，host属性是一个IP地址，而不是一个DNS 名称
- (void)onSocket:(AsyncSocket *)sock didConnectToHost:(NSString *)host port:(UInt16)port;

/**
 * Called when a socket has completed reading the requested data into memory.
 * Not called if there is an error.
**/
//当socket已完成所要求的数据读入内存时调用，如果有错误则不调用
- (void)onSocket:(AsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag;

/**
 * Called when a socket has read in data, but has not yet completed the read.
 * This would occur if using readToData: or readToLength: methods.
 * It may be used to for things such as updating progress bars.
**/
//当一个socket读取数据，但尚未完成读操作的时候调用，如果使用 readToData: or readToLength: 方法 会发生,可以被用来更新进度条等东西
- (void)onSocket:(AsyncSocket *)sock didReadPartialDataOfLength:(NSUInteger)partialLength tag:(long)tag;

/**
 * Called when a socket has completed writing the requested data. Not called if there is an error.
**/
//当一个socket已完成请求数据的写入时候调用
- (void)onSocket:(AsyncSocket *)sock didWriteDataWithTag:(long)tag;

/**
 * Called when a socket has written some data, but has not yet completed the entire write.
 * It may be used to for things such as updating progress bars.
**/
//当一个socket写入一些数据，但还没有完成整个写入时调用，它可以用来更新进度条等东西
- (void)onSocket:(AsyncSocket *)sock didWritePartialDataOfLength:(NSUInteger)partialLength tag:(long)tag;

/**
 * Called if a read operation has reached its timeout without completing.
 * This method allows you to optionally extend the timeout.
 * If you return a positive time interval (> 0) the read's timeout will be extended by the given amount.
 * If you don't implement this method, or return a non-positive time interval (<= 0) the read will timeout as usual.
 * 
 * The elapsed parameter is the sum of the original timeout, plus any additions previously added via this method.
 * The length parameter is the number of bytes that have been read so far for the read operation.
 * 
 * Note that this method may be called multiple times for a single read if you return positive numbers.
**/
//使用读操作已超时但还没完成时调用，此方法允许随意延迟超时，如果返回一个正的时间间隔，读取的超时将有一定量的扩展，如果不实现这个方法，或会像往常一样 返回一个负的时间间隔，elapsed参数是  原超时的总和，加上先前通过这种方法添加的任何补充， length参数是 读操作到目前为止已读取的字节数， 注意，如果返回正数的话，这个方法可能被一个单独的读取多次调用
- (NSTimeInterval)onSocket:(AsyncSocket *)sock
  shouldTimeoutReadWithTag:(long)tag
                   elapsed:(NSTimeInterval)elapsed
                 bytesDone:(NSUInteger)length;

/**
 * Called if a write operation has reached its timeout without completing.
 * This method allows you to optionally extend the timeout.
 * If you return a positive time interval (> 0) the write's timeout will be extended by the given amount.
 * If you don't implement this method, or return a non-positive time interval (<= 0) the write will timeout as usual.
 * 
 * The elapsed parameter is the sum of the original timeout, plus any additions previously added via this method.
 * The length parameter is the number of bytes that have been written so far for the write operation.
 * 
 * Note that this method may be called multiple times for a single write if you return positive numbers.
**/
//如果一个写操作已达到其超时但还没完成时调用，同上
- (NSTimeInterval)onSocket:(AsyncSocket *)sock
 shouldTimeoutWriteWithTag:(long)tag
                   elapsed:(NSTimeInterval)elapsed
                 bytesDone:(NSUInteger)length;

/**
 * Called after the socket has successfully completed SSL/TLS negotiation.
 * This method is not called unless you use the provided startTLS method.
 * 
 * If a SSL/TLS negotiation fails (invalid certificate, etc) then the socket will immediately close,
 * and the onSocket:willDisconnectWithError: delegate method will be called with the specific SSL error code.
**/
//在socket成功完成ssl/tls协商时调用，此方法除非你使用提供startTLS方法时候才调用，
//如果ssl/tls是无效的证书，socket将会立即关闭，onSocket:willDisconnectWithError:代理方法竟会与特定的ssl错误代码一起调用
- (void)onSocketDidSecure:(AsyncSocket *)sock;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface AsyncSocket : NSObject
{
	CFSocketNativeHandle theNativeSocket4;
	CFSocketNativeHandle theNativeSocket6;
	
	CFSocketRef theSocket4;            // IPv4 accept or connect socket
	CFSocketRef theSocket6;            // IPv6 accept or connect socket
	
	CFReadStreamRef theReadStream;
	CFWriteStreamRef theWriteStream;

	CFRunLoopSourceRef theSource4;     // For theSocket4
	CFRunLoopSourceRef theSource6;     // For theSocket6
	CFRunLoopRef theRunLoop;
	CFSocketContext theContext;
	NSArray *theRunLoopModes;
	
	NSTimer *theConnectTimer;

	NSMutableArray *theReadQueue;
	AsyncReadPacket *theCurrentRead;
	NSTimer *theReadTimer;
	NSMutableData *partialReadBuffer;
	
	NSMutableArray *theWriteQueue;
	AsyncWritePacket *theCurrentWrite;
	NSTimer *theWriteTimer;

	id theDelegate;
	UInt16 theFlags;
	
	long theUserData;
}

- (id)init;
- (id)initWithDelegate:(id)delegate;
- (id)initWithDelegate:(id)delegate userData:(long)userData;

/* String representation is long but has no "\n". */
- (NSString *)description;

/**
 * Use "canSafelySetDelegate" to see if there is any pending business (reads and writes) with the current delegate
 * before changing it.  It is, of course, safe to change the delegate before connecting or accepting connections.
**/
- (id)delegate;
- (BOOL)canSafelySetDelegate;
- (void)setDelegate:(id)delegate;

/* User data can be a long, or an id or void * cast to a long. */
- (long)userData;
- (void)setUserData:(long)userData;

/* Don't use these to read or write. And don't close them either! */
- (CFSocketRef)getCFSocket;
- (CFReadStreamRef)getCFReadStream;
- (CFWriteStreamRef)getCFWriteStream;

// Once one of the accept or connect methods are called, the AsyncSocket instance is locked in
// and the other accept/connect methods can't be called without disconnecting the socket first.
// If the attempt fails or times out, these methods either return NO or
// call "onSocket:willDisconnectWithError:" and "onSockedDidDisconnect:".

// When an incoming connection is accepted, AsyncSocket invokes several delegate methods.
// These methods are (in chronological order):
// 1. onSocket:didAcceptNewSocket:
// 2. onSocket:wantsRunLoopForNewSocket:
// 3. onSocketWillConnect:
// 
// Your server code will need to retain the accepted socket (if you want to accept it).
// The best place to do this is probably in the onSocket:didAcceptNewSocket: method.
// 
// After the read and write streams have been setup for the newly accepted socket,
// the onSocket:didConnectToHost:port: method will be called on the proper run loop.
// 
// Multithreading Note: If you're going to be moving the newly accepted socket to another run
// loop by implementing onSocket:wantsRunLoopForNewSocket:, then you should wait until the
// onSocket:didConnectToHost:port: method before calling read, write, or startTLS methods.
// Otherwise read/write events are scheduled on the incorrect runloop, and chaos may ensue.

/**
 * Tells the socket to begin listening and accepting connections on the given port.
 * When a connection comes in, the AsyncSocket instance will call the various delegate methods (see above).
 * The socket will listen on all available interfaces (e.g. wifi, ethernet, etc)
**/
//告诉socket开始听取和接受指定端口上的连接，当一个连接到来的时候，AsyncSocket实例将调用各种委托方法，socket将听取所有可用的接口(wifi,以太网等)
- (BOOL)acceptOnPort:(UInt16)port error:(NSError **)errPtr;

/**
 * This method is the same as acceptOnPort:error: with the additional option
 * of specifying which interface to listen on. So, for example, if you were writing code for a server that
 * has multiple IP addresses, you could specify which address you wanted to listen on.  Or you could use it
 * to specify that the socket should only accept connections over ethernet, and not other interfaces such as wifi.
 * You may also use the special strings "localhost" or "loopback" to specify that
 * the socket only accept connections from the local machine.
 * 
 * To accept connections on any interface pass nil, or simply use the acceptOnPort:error: method.
**/
- (BOOL)acceptOnInterface:(NSString *)interface port:(UInt16)port error:(NSError **)errPtr;

/**
 * Connects to the given host and port.
 * The host may be a domain name (e.g. "deusty.com") or an IP address string (e.g. "192.168.0.2")
**/
//连接给定的主机和端口，主机hostname可以是域名或者是Ip地址
- (BOOL)connectToHost:(NSString *)hostname onPort:(UInt16)port error:(NSError **)errPtr;

/**
 * This method is the same as connectToHost:onPort:error: with an additional timeout option.
 * To not time out use a negative time interval, or simply use the connectToHost:onPort:error: method.
**/
- (BOOL)connectToHost:(NSString *)hostname
			   onPort:(UInt16)port
		  withTimeout:(NSTimeInterval)timeout
				error:(NSError **)errPtr;

/**
 * Connects to the given address, specified as a sockaddr structure wrapped in a NSData object.
 * For example, a NSData object returned from NSNetService's addresses method.
 * 
 * If you have an existing struct sockaddr you can convert it to a NSData object like so:
 * struct sockaddr sa  -> NSData *dsa = [NSData dataWithBytes:&remoteAddr length:remoteAddr.sa_len];
 * struct sockaddr *sa -> NSData *dsa = [NSData dataWithBytes:remoteAddr length:remoteAddr->sa_len];
**/
//连接到一个给定的地址，指定一个sockaddr结构包裹住一个NSData对象,例如，NSData对象从NSNetService的地址方法返回,如果有一个现有的sockaddr结构，可以将它转换到一个NSData对象，像这样：
//struct sockaddr sa  -> NSData *dsa = [NSData dataWithBytes:&remoteAddr length:remoteAddr.sa_len];
//struct sockaddr *sa -> NSData *dsa = [NSData dataWithBytes:remoteAddr length:remoteAddr->sa_len];
- (BOOL)connectToAddress:(NSData *)remoteAddr error:(NSError **)errPtr;

/**
 * This method is the same as connectToAddress:error: with an additional timeout option.
 * To not time out use a negative time interval, or simply use the connectToAddress:error: method.
**/
- (BOOL)connectToAddress:(NSData *)remoteAddr withTimeout:(NSTimeInterval)timeout error:(NSError **)errPtr;

- (BOOL)connectToAddress:(NSData *)remoteAddr
     viaInterfaceAddress:(NSData *)interfaceAddr
             withTimeout:(NSTimeInterval)timeout
                   error:(NSError **)errPtr;

/**
 * Disconnects immediately. Any pending reads or writes are dropped.
 * If the socket is not already disconnected, the onSocketDidDisconnect delegate method
 * will be called immediately, before this method returns.
 * 
 * Please note the recommended way of releasing an AsyncSocket instance (e.g. in a dealloc method)
 * [asyncSocket setDelegate:nil];
 * [asyncSocket disconnect];
 * [asyncSocket release];
**/
//立即断开，任何未处理的读或写都将被丢弃
//如果socket还没有断开，在这个方法返回之前，onSocketDidDisconnect 委托方法将会被立即调用
//注意推荐释放AsyncSocket实例的方式：
//[asyncSocket setDelegate:nil];
//[asyncSocket disconnect];
//[asyncSocket release];
- (void)disconnect;

/**
 * Disconnects after all pending reads have completed.
 * After calling this, the read and write methods will do nothing.
 * The socket will disconnect even if there are still pending writes.
**/
//在已经完成了所有悬而未决的读取时 断开,在调用之后，读取和写入方法将无用,socket将断开 即使仍有写入的东西
- (void)disconnectAfterReading;

/**
 * Disconnects after all pending writes have completed.
 * After calling this, the read and write methods will do nothing.
 * The socket will disconnect even if there are still pending reads.
**/
- (void)disconnectAfterWriting;

/**
 * Disconnects after all pending reads and writes have completed.
 * After calling this, the read and write methods will do nothing.
**/
- (void)disconnectAfterReadingAndWriting;

/* Returns YES if the socket and streams are open, connected, and ready for reading and writing. */
- (BOOL)isConnected;

/**
 * Returns the local or remote host and port to which this socket is connected, or nil and 0 if not connected.
 * The host will be an IP address.
**/
//返回本地和远程主机和端口给连接的socket，如果没有连接会返回nil或0,主机将会是一个IP地址
- (NSString *)connectedHost;
- (UInt16)connectedPort;
- (NSString *)localHost;
- (UInt16)localPort;

/**
 * Returns the local or remote address to which this socket is connected,
 * specified as a sockaddr structure wrapped in a NSData object.
 * 
 * See also the connectedHost, connectedPort, localHost and localPort methods.
**/
//返回本地和远程的地址给连接的socket，指定一个socketaddr结构包裹在一个NSData对象
//
//
//readData和writeData方法不会是block(它们是异步的)
//当读完成 onSocket:didReadData:withTag: 委托方法时调用
//当写完成 onSocket:didWriteDataWithTag: 委托方法时调用
//可以选择任何读/写操作的超时设置(为了不超时，使用负时间间隔。）
//                 如果读/写操作超时，相应的 onSocket:shouldTimeout...委托方法被调用去选择性地允许我们去延长超时
//                 超时后，onSocket:willDisconnectWithError: 方法被调用,紧接着是 onSocketDidDisconnect
//                 tag是为了方便，可以使用它作为数组的索引、步数、state id 、指针等

- (NSData *)connectedAddress;
- (NSData *)localAddress;

/**
 * Returns whether the socket is IPv4 or IPv6.
 * An accepting socket may be both.
**/
- (BOOL)isIPv4;
- (BOOL)isIPv6;

// The readData and writeData methods won't block (they are asynchronous).
// 
// When a read is complete the onSocket:didReadData:withTag: delegate method is called.
// When a write is complete the onSocket:didWriteDataWithTag: delegate method is called.
// 
// You may optionally set a timeout for any read/write operation. (To not timeout, use a negative time interval.)
// If a read/write opertion times out, the corresponding "onSocket:shouldTimeout..." delegate method
// is called to optionally allow you to extend the timeout.
// Upon a timeout, the "onSocket:willDisconnectWithError:" method is called, followed by "onSocketDidDisconnect".
// 
// The tag is for your convenience.
// You can use it as an array index, step number, state id, pointer, etc.

/**
 * Reads the first available bytes that become available on the socket.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
**/
//读取socket上第一次成为可用的字节,如果timeout值是负数的，读操作将不使用timeout
- (void)readDataWithTimeout:(NSTimeInterval)timeout tag:(long)tag;

/**
 * Reads the first available bytes that become available on the socket.
 * The bytes will be appended to the given byte buffer starting at the given offset.
 * The given buffer will automatically be increased in size if needed.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * If the buffer if nil, the socket will create a buffer for you.
 * 
 * If the bufferOffset is greater than the length of the given buffer,
 * the method will do nothing, and the delegate will not be called.
 * 
 * If you pass a buffer, you must not alter it in any way while AsyncSocket is using it.
 * After completion, the data returned in onSocket:didReadData:withTag: will be a subset of the given buffer.
 * That is, it will reference the bytes that were appended to the given buffer.
**/
//读取socket上第一次成为可用的字节
//字节将被追加到给定的字节缓冲区，从给定的偏移量开始
//如果需要，给定的缓冲区大小将会自动增加
//如果timeout值是负数的，读操作将不使用timeout
//如果缓冲区为空，socket会为我们创建一个缓冲区
//如果bufferOffset是大于给定的缓冲区的长度，该方法将无用，委托将不会被调用
//如果你传递一个缓冲区，当AsyncSocket在使用它的时候你不能以任何方式改变它
//完成之后，onSocket:didReadData:withTag 返回的数据将是一个给定的缓冲区的子集
//也就是说，它将会被引用到被追加的给定的缓冲区的字节
- (void)readDataWithTimeout:(NSTimeInterval)timeout
					 buffer:(NSMutableData *)buffer
			   bufferOffset:(NSUInteger)offset
						tag:(long)tag;

/**
 * Reads the first available bytes that become available on the socket.
 * The bytes will be appended to the given byte buffer starting at the given offset.
 * The given buffer will automatically be increased in size if needed.
 * A maximum of length bytes will be read.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * If the buffer if nil, a buffer will automatically be created for you.
 * If maxLength is zero, no length restriction is enforced.
 * 
 * If the bufferOffset is greater than the length of the given buffer,
 * the method will do nothing, and the delegate will not be called.
 * 
 * If you pass a buffer, you must not alter it in any way while AsyncSocket is using it.
 * After completion, the data returned in onSocket:didReadData:withTag: will be a subset of the given buffer.
 * That is, it will reference the bytes that were appended to the given buffer.
**/
- (void)readDataWithTimeout:(NSTimeInterval)timeout
                     buffer:(NSMutableData *)buffer
               bufferOffset:(NSUInteger)offset
                  maxLength:(NSUInteger)length
                        tag:(long)tag;

/**
 * Reads the given number of bytes.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * 
 * If the length is 0, this method does nothing and the delegate is not called.
**/
//读取给定的字节数，如果length为0，方法将无用，委托将不会被调用
- (void)readDataToLength:(NSUInteger)length withTimeout:(NSTimeInterval)timeout tag:(long)tag;

/**
 * Reads the given number of bytes.
 * The bytes will be appended to the given byte buffer starting at the given offset.
 * The given buffer will automatically be increased in size if needed.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * If the buffer if nil, a buffer will automatically be created for you.
 * 
 * If the length is 0, this method does nothing and the delegate is not called.
 * If the bufferOffset is greater than the length of the given buffer,
 * the method will do nothing, and the delegate will not be called.
 * 
 * If you pass a buffer, you must not alter it in any way while AsyncSocket is using it.
 * After completion, the data returned in onSocket:didReadData:withTag: will be a subset of the given buffer.
 * That is, it will reference the bytes that were appended to the given buffer.
**/
//读取给定的字节数,在给定的偏移开始，字节将被追加到给定的字节缓冲区
- (void)readDataToLength:(NSUInteger)length
             withTimeout:(NSTimeInterval)timeout
                  buffer:(NSMutableData *)buffer
            bufferOffset:(NSUInteger)offset
                     tag:(long)tag;

/**
 * Reads bytes until (and including) the passed "data" parameter, which acts as a separator.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * 
 * If you pass nil or zero-length data as the "data" parameter,
 * the method will do nothing, and the delegate will not be called.
 * 
 * To read a line from the socket, use the line separator (e.g. CRLF for HTTP, see below) as the "data" parameter.
 * Note that this method is not character-set aware, so if a separator can occur naturally as part of the encoding for
 * a character, the read will prematurely end.
**/
//读取字节直到(包括)传入的作为分隔的"data"参数
//如果传递0或者0长度的数据，"data"参数，该方法将无用，委托将不会被调用
//从socket读取一行，使用"data"参数作为行的分隔符 (如HTTP的CRLF)
//注意，此方法不是字符集，因此，如果一个分隔符出现，它自然可以作为进行编码的一部分，读取将提前结束

- (void)readDataToData:(NSData *)data withTimeout:(NSTimeInterval)timeout tag:(long)tag;

/**
 * Reads bytes until (and including) the passed "data" parameter, which acts as a separator.
 * The bytes will be appended to the given byte buffer starting at the given offset.
 * The given buffer will automatically be increased in size if needed.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * If the buffer if nil, a buffer will automatically be created for you.
 * 
 * If the bufferOffset is greater than the length of the given buffer,
 * the method will do nothing, and the delegate will not be called.
 * 
 * If you pass a buffer, you must not alter it in any way while AsyncSocket is using it.
 * After completion, the data returned in onSocket:didReadData:withTag: will be a subset of the given buffer.
 * That is, it will reference the bytes that were appended to the given buffer.
 * 
 * To read a line from the socket, use the line separator (e.g. CRLF for HTTP, see below) as the "data" parameter.
 * Note that this method is not character-set aware, so if a separator can occur naturally as part of the encoding for
 * a character, the read will prematurely end.
**/
//读取字节直到(包括)传入的作为分隔的“data”参数，在给定的偏移量开始，字节将被追加到给定的字节缓冲区。
//从socket读取一行，使用"data"参数作为行的分隔符(如HTTP的CRLF)
- (void)readDataToData:(NSData *)data
           withTimeout:(NSTimeInterval)timeout
                buffer:(NSMutableData *)buffer
          bufferOffset:(NSUInteger)offset
                   tag:(long)tag;

/**
 * Reads bytes until (and including) the passed "data" parameter, which acts as a separator.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * 
 * If maxLength is zero, no length restriction is enforced.
 * Otherwise if maxLength bytes are read without completing the read,
 * it is treated similarly to a timeout - the socket is closed with a AsyncSocketReadMaxedOutError.
 * The read will complete successfully if exactly maxLength bytes are read and the given data is found at the end.
 * 
 * If you pass nil or zero-length data as the "data" parameter,
 * the method will do nothing, and the delegate will not be called.
 * If you pass a maxLength parameter that is less than the length of the data parameter,
 * the method will do nothing, and the delegate will not be called.
 * 
 * To read a line from the socket, use the line separator (e.g. CRLF for HTTP, see below) as the "data" parameter.
 * Note that this method is not character-set aware, so if a separator can occur naturally as part of the encoding for
 * a character, the read will prematurely end.
**/
- (void)readDataToData:(NSData *)data withTimeout:(NSTimeInterval)timeout maxLength:(NSUInteger)length tag:(long)tag;

/**
 * Reads bytes until (and including) the passed "data" parameter, which acts as a separator.
 * The bytes will be appended to the given byte buffer starting at the given offset.
 * The given buffer will automatically be increased in size if needed.
 * A maximum of length bytes will be read.
 * 
 * If the timeout value is negative, the read operation will not use a timeout.
 * If the buffer if nil, a buffer will automatically be created for you.
 * 
 * If maxLength is zero, no length restriction is enforced.
 * Otherwise if maxLength bytes are read without completing the read,
 * it is treated similarly to a timeout - the socket is closed with a AsyncSocketReadMaxedOutError.
 * The read will complete successfully if exactly maxLength bytes are read and the given data is found at the end.
 * 
 * If you pass a maxLength parameter that is less than the length of the data parameter,
 * the method will do nothing, and the delegate will not be called.
 * If the bufferOffset is greater than the length of the given buffer,
 * the method will do nothing, and the delegate will not be called.
 * 
 * If you pass a buffer, you must not alter it in any way while AsyncSocket is using it.
 * After completion, the data returned in onSocket:didReadData:withTag: will be a subset of the given buffer.
 * That is, it will reference the bytes that were appended to the given buffer.
 * 
 * To read a line from the socket, use the line separator (e.g. CRLF for HTTP, see below) as the "data" parameter.
 * Note that this method is not character-set aware, so if a separator can occur naturally as part of the encoding for
 * a character, the read will prematurely end.
**/
- (void)readDataToData:(NSData *)data
           withTimeout:(NSTimeInterval)timeout
                buffer:(NSMutableData *)buffer
          bufferOffset:(NSUInteger)offset
             maxLength:(NSUInteger)length
                   tag:(long)tag;

/**
 * Writes data to the socket, and calls the delegate when finished.
 * 
 * If you pass in nil or zero-length data, this method does nothing and the delegate will not be called.
 * If the timeout value is negative, the write operation will not use a timeout.
**/
//将data写入socket，当完成的时候委托被调用
- (void)writeData:(NSData *)data withTimeout:(NSTimeInterval)timeout tag:(long)tag;

/**
 * Returns progress of current read or write, from 0.0 to 1.0, or NaN if no read/write (use isnan() to check).
 * "tag", "done" and "total" will be filled in if they aren't NULL.
**/
//返回当前读或写的进度，从0.0 到 1.0 或者 如果没有读/写的时候返回Nan(使用isNan来检查)
//tag、done、total如果不为空的话，它们将会被填补
- (float)progressOfReadReturningTag:(long *)tag bytesDone:(NSUInteger *)done total:(NSUInteger *)total;
- (float)progressOfWriteReturningTag:(long *)tag bytesDone:(NSUInteger *)done total:(NSUInteger *)total;

/**
 * Secures the connection using SSL/TLS.
 * 
 * This method may be called at any time, and the TLS handshake will occur after all pending reads and writes
 * are finished. This allows one the option of sending a protocol dependent StartTLS message, and queuing
 * the upgrade to TLS at the same time, without having to wait for the write to finish.
 * Any reads or writes scheduled after this method is called will occur over the secured connection.
 * 
 * The possible keys and values for the TLS settings are well documented.
 * Some possible keys are:
 * - kCFStreamSSLLevel
 * - kCFStreamSSLAllowsExpiredCertificates
 * - kCFStreamSSLAllowsExpiredRoots
 * - kCFStreamSSLAllowsAnyRoot
 * - kCFStreamSSLValidatesCertificateChain
 * - kCFStreamSSLPeerName
 * - kCFStreamSSLCertificates
 * - kCFStreamSSLIsServer
 * 
 * Please refer to Apple's documentation for associated values, as well as other possible keys.
 * 
 * If you pass in nil or an empty dictionary, the default settings will be used.
 * 
 * The default settings will check to make sure the remote party's certificate is signed by a
 * trusted 3rd party certificate agency (e.g. verisign) and that the certificate is not expired.
 * However it will not verify the name on the certificate unless you
 * give it a name to verify against via the kCFStreamSSLPeerName key.
 * The security implications of this are important to understand.
 * Imagine you are attempting to create a secure connection to MySecureServer.com,
 * but your socket gets directed to MaliciousServer.com because of a hacked DNS server.
 * If you simply use the default settings, and MaliciousServer.com has a valid certificate,
 * the default settings will not detect any problems since the certificate is valid.
 * To properly secure your connection in this particular scenario you
 * should set the kCFStreamSSLPeerName property to "MySecureServer.com".
 * If you do not know the peer name of the remote host in advance (for example, you're not sure
 * if it will be "domain.com" or "www.domain.com"), then you can use the default settings to validate the
 * certificate, and then use the X509Certificate class to verify the issuer after the socket has been secured.
 * The X509Certificate class is part of the CocoaAsyncSocket open source project.
**/
//确保使用ssl/tls连接
//这方法可被随时调用，tls握手将会发生在所有悬而未决的读/写完成之后。这紧跟着一个发送依赖 StartTLS消息的协议选项，在排队升级到TLS的同一时间，而不必等待写入完成。在这个方法被调用后,任何读写计划 将会发生在安全链接
//对于可能的keys和TLS设置的值是有据可查的
//一些可能的keys是:
//* - kCFStreamSSLLevel
//* - kCFStreamSSLAllowsExpiredCertificates
//* - kCFStreamSSLAllowsExpiredRoots
//* - kCFStreamSSLAllowsAnyRoot
//* - kCFStreamSSLValidatesCertificateChain
//* - kCFStreamSSLPeerName
//* - kCFStreamSSLCertificates
//* - kCFStreamSSLIsServer
//
//如果你传递空或者空字典，将使用默认的字典
//默认设置将检查以确保由签署可信的第三方证书机构和没有过期的远程连接的证书
//然而，它不会验证证书上的名字，除非你给它一个名字，通过kCFStreamSSLPeerName键去验证
//这对安全的影响是重要的理解
//想象一下你正试图创建一个到MySecureServer.com的安全连接，但因为一个被攻击的DNS服务器，所以你的socket被定向到MaliciousServer.com
//如果你只是使用默认设置，MaliciousServer.com 有一个有效的证书
//默认设置将无法监测到任何问题，因为证书是有效的
//在这个特殊的情况下，要妥善保护你的连接，应设置kCFStreamSSLPeerName性质为MySecureServer.com.
//如 果事前你不知道对等的名字的远程主机(例如，你不确认它是domain.com" or "www.domain.com")，那么你可以使用默认设置来验证证书，然后在获得验证的发行后使用X509Certificate类来验 证,X509Certificate类的CocoaAsyncSocket开源项目的一部分
- (void)startTLS:(NSDictionary *)tlsSettings;

/**
 * For handling readDataToData requests, data is necessarily read from the socket in small increments.
 * The performance can be much improved by allowing AsyncSocket to read larger chunks at a time and
 * store any overflow in a small internal buffer.
 * This is termed pre-buffering, as some data may be read for you before you ask for it.
 * If you use readDataToData a lot, enabling pre-buffering will result in better performance, especially on the iPhone.
 * 
 * The default pre-buffering state is controlled by the DEFAULT_PREBUFFERING definition.
 * It is highly recommended one leave this set to YES.
 * 
 * This method exists in case pre-buffering needs to be disabled by default for some unforeseen reason.
 * In that case, this method exists to allow one to easily enable pre-buffering when ready.
**/
//对于处理readDataToData请求，数据是必须从socket以小增量的方式读取出来的
//性能通过允许AsyncSocket去一次性读大块的数据和存储任何一个小的内部缓冲区溢出的东西来大大提高
//这被称为预缓冲，就好像一些数据在你要求它之前就可能被读取出来
//如果你经常使用readDataToData，使用预缓冲会有更好的性能，尤其是在iphone上
//默认的预缓冲状态是由DEFAULT_PREBUFFERING 定义控制的，强烈建议设置其为yes
//这方法存在一些预缓冲需要一些不可预见的原因被默认禁用的情况，这时，这种方法存在允许当就绪时，可轻松启用预缓冲
- (void)enablePreBuffering;

/**
 * When you create an AsyncSocket, it is added to the runloop of the current thread.
 * So for manually created sockets, it is easiest to simply create the socket on the thread you intend to use it.
 * 
 * If a new socket is accepted, the delegate method onSocket:wantsRunLoopForNewSocket: is called to
 * allow you to place the socket on a separate thread. This works best in conjunction with a thread pool design.
 * 
 * If, however, you need to move the socket to a separate thread at a later time, this
 * method may be used to accomplish the task.
 * 
 * This method must be called from the thread/runloop the socket is currently running on.
 * 
 * Note: After calling this method, all further method calls to this object should be done from the given runloop.
 * Also, all delegate calls will be sent on the given runloop.
**/

//当你创建一个AsyncSocket，它被添加到当前线程runloop
//对于手动创建的socket，在线程上你打算使用它，它是最容易简单的创建的线程上的socket
//当一个新的socket被接受，委托方法 onSocket:wantsRunLoopForNewSocket 会被调用 允许你在一个单独的线程上放置socket，这个工作最好结合在同一个线程池设计
//如果，但是，在一个单独的线程上，在之后的时间，你需要移动一个socket，这个方法可以用来完成任务
//此方法必须从 当前运行的 线程/runloop 的socket 调用
//注意：此方法调用后，所有进一步的方法应该从给定的runloop上调用这个对象
//此外，所有委托调用将会发送到给定的runloop
- (BOOL)moveToRunLoop:(NSRunLoop *)runLoop;

/**
 * Allows you to configure which run loop modes the socket uses.
 * The default set of run loop modes is NSDefaultRunLoopMode.
 * 
 * If you'd like your socket to continue operation during other modes, you may want to add modes such as
 * NSModalPanelRunLoopMode or NSEventTrackingRunLoopMode. Or you may simply want to use NSRunLoopCommonModes.
 * 
 * Accepted sockets will automatically inherit the same run loop modes as the listening socket.
 * 
 * Note: NSRunLoopCommonModes is defined in 10.5. For previous versions one can use kCFRunLoopCommonModes.
**/
//一下三个方法
//允许你配置 socket 使用的 运行循环模式
//运行循环模式设置默认是NSRunLoopCommonModes
//如果你想你的socket 在其他模式下继续操作，你可能需要添加模式 NSModalPanelRunLoopMode 或者 NSEventTrackingRunLoopMode ,或者你可能只想使用 NSRunLoopCommonModes
//可接受的socket将自动 继承相同的运行循环模式就像侦听socket
//注意：NSRunLoopCommonModes 定义在10.5，对于之前的版本可使用 kCFRunLoopCommonModes
- (BOOL)setRunLoopModes:(NSArray *)runLoopModes;
- (BOOL)addRunLoopMode:(NSString *)runLoopMode;
- (BOOL)removeRunLoopMode:(NSString *)runLoopMode;

/**
 * Returns the current run loop modes the AsyncSocket instance is operating in.
 * The default set of run loop modes is NSDefaultRunLoopMode.
**/
//返回当前正在运行的循环模式的AsyncSocket实例， run loop modes的默认设置是NSDefaultRunLoopMode
- (NSArray *)runLoopModes;

/**
 * In the event of an error, this method may be called during onSocket:willDisconnectWithError: to read
 * any data that's left on the socket.
**/
//一个错误的事件，在 onSocket:willDisconnectWithError: 将会被调用 去读取留在socket上的任何数据
- (NSData *)unreadData;

/* A few common line separators, for use with the readDataToData:... methods. */

//一些常见的分隔符，为了去使用  readDataToData:..  方法
+ (NSData *)CRLFData;   // 0x0D0A
+ (NSData *)CRData;     // 0x0D
+ (NSData *)LFData;     // 0x0A
+ (NSData *)ZeroData;   // 0x00

@end
