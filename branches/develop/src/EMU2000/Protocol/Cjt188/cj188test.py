
import socket
import time
import struct


def bytesToList(b: bytes):
    blist = []
    for i in b:
        blist.append(int(i))
    return blist


def cs(lst: list, a: int, b: int):
    sum = 0
    for i in range(a, b + 1):
        sum += lst[i]
    return sum % 256


def int2BCD(num: int):
    return num // 10 * 16 + num % 10

def hexmsg(data):
    str_ = ''
    for i in range(len(data)):
        a = hex(data[i]).replace('0x', '').upper()
        if len(a) == 1:
            str_ += ' 0' + a[0]
        else:
            str_ += ' ' + a[0] + a[1]
    return str_


efs = 3
sk = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)
sk.connect(("192.168.1.253", 20001))
while True:
    sendmsg = []
    smsg = b''
    recvmsg = bytesToList(sk.recv(1024))
    print(hexmsg(recvmsg))
    # print(cs(recvmsg, efs, len(recvmsg) - 3))
    for i in recvmsg:
        sendmsg.append(i)
    if recvmsg[11 + efs] == 31 and recvmsg[12 + efs] == 144:  # 判断1F 90 读数据
        sendmsg[9 + efs] = 0x81 # 修改命字
        sendmsg[-2:-2] = [0x10, 0x00, 0x10, 0x00, 0x2C,0x10, 0x00, 0x10, 0x00, 0x2C] # 插入数据
        sendmsg[-2:-2] = [int2BCD(time.localtime().tm_sec), int2BCD(time.localtime().tm_min),
                          int2BCD(time.localtime().tm_hour), int2BCD(time.localtime().tm_mday),
                          int2BCD(time.localtime().tm_mon), int2BCD(time.localtime().tm_year % 100),
                          int2BCD(time.localtime().tm_year // 100)]   # 获取当前时间并且转换为bcd码
        sendmsg[-2:-2] = [0x00, 0xff] # 插入装置状态
        sendmsg[10 + efs] = len(sendmsg) - efs - 13  # 修改数据域长度的

    elif recvmsg[11 + efs] == 21 and recvmsg[12 + efs] == 160:  #判断15 A0 对时
        sendmsg[9 + efs] = 0x84  # 修改命令字，对时的回复和下发报文一致

    sendmsg[-2] = cs(sendmsg, efs, len(sendmsg) - 3)  # 计算校验和
    print(hexmsg(sendmsg))  #16 进制打印
    smsg = struct.pack("%dB" % (len(sendmsg)), *sendmsg)

    # for i in sendmsg:
    #     a = struct.pack('=B', i)
    #     smsg += a


    print("=========================")
    sk.send(smsg)