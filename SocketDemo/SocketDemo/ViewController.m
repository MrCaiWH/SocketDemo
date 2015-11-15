//
//  ViewController.m
//  SocketDemo
//
//  Created by 蔡万鸿 on 15/11/15.
//  Copyright © 2015年 黄花菜. All rights reserved.
//

#import "ViewController.h"
#import "AsyncSocket.h"

@interface ViewController ()<AsyncSocketDelegate>
{
    AsyncSocket *socket;
}
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    socket=[[AsyncSocket alloc] initWithDelegate:self];
    [socket connectToHost:@"www.baidu.com" onPort:80 error:nil];
    
    [socket readDataToLength:50 withTimeout:5 tag:1];
    [socket readDataToLength:50 withTimeout:5 tag:2];
    [socket writeData:[@"GET / HTTP/1.1\n\n" dataUsingEncoding:NSUTF8StringEncoding] withTimeout:3 tag:1];

    
}

- (void)onSocket:(AsyncSocket *)sock didConnectToHost:(NSString *)host port:(UInt16)port{
    NSLog(@"did connect to host");
}

- (void)onSocket:(AsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag{
    NSLog(@"did read data");
    NSString* message = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    NSLog(@"message is: \n%@",message);
}

- (void)onSocketDidDisconnect:(AsyncSocket *)sock{
    NSLog(@"socket did disconnect");
}

@end
