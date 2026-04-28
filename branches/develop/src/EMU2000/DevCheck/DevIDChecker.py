import socket
import threading
import time
import csv
import sys
import os


def add_dev(devid: str, addr: str):
    if devid in data.keys():
        data[devid].append(addr)
        data[devid] = list(set(data[devid]))
    else:
        data[devid] = [addr]


def add_dev2(addr, devlist: list):
    remove_list = []
    for i in range(len(devlist)):
        try:
            devlist[i] = int(devlist[i])
        except:
            remove_list.append(devlist[i])
    for i in remove_list:
        devlist.remove(i)
    devlist.sort()
    if devlist:
        if addr not in data2.keys():
            data2[addr] = list(set(devlist))
            data2[addr].sort()
        else:
            data2[addr] = list(set(devlist + data2[addr]))
            data2[addr].sort()


def dev_id_request():
    client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    client.sendto('devid'.encode('utf-8'), ('255.255.255.255', port))


def udp_recv():
    def func():
        global count
        while True:
            try:
                msg, addr = server.recvfrom(3072)
            except:
                continue
            # print(msg.decode("utf-8"), addr)
            devlist = msg.decode("utf-8").split(",")
            add_dev2(addr[0], devlist)
            # print(data2)
            for i in devlist:
                add_dev(i, addr[0])
            count += 1
            # print(data)

    t = threading.Thread(target=func)
    t.setDaemon(True)
    t.start()


def showdepulication():
    client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client.sendto(''.encode('utf-8'), ('127.0.0.1', port))
    msg = ''
    for i in data.keys():
        if len(data[i]) > 1:
            print("装置号 ", i, " 在以下地址上重复:", end="")
            msg = msg + "装置号 " + str(i) + " 在以下地址上重复:  "
            for j in data[i]:
                print(" ", j, end="")
                msg = msg + j + "  "
            print("")
            msg += '\n'
    if not msg:
        print("没有装置号重复")
    else:
        with open(currentpath + '/depulication.txt', mode="w") as f:
            f.write(msg)


def write_cvs():
    wlist = []
    for key in data2.keys():
        wlist.append([key] + data2[key])
    with open(currentpath + '/devlist.csv', mode='w', newline="") as f:
        writer = csv.writer(f)
        writer.writerows(wlist)


try:
    port = int(sys.argv[1])
    sleeptime = float(sys.argv[2])
except :
    print("参数不合法，使用默认参数")
    port = 16000
    sleeptime = 5.0
currentpath = os.path.dirname(sys.argv[0])
if os.path.exists(currentpath + '/depulication.txt'):
    os.remove(currentpath + '/depulication.txt')
count = 0
data = {}
data2 = {}
server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
server.bind(('0.0.0.0', port))
print('在UDP端口 ', port, ' 上开始监听')
udp_recv()
print("请求已发送")
dev_id_request()
print("正在分析，请稍", sleeptime, "秒")
time.sleep(sleeptime)
server.close()
print(count - 1, " 条信息被收到")
print("显示重复的装置号")
print('==================================')
showdepulication()
print('==================================')
print('正在创建装置列表文件...')
try:
    write_cvs()
except OSError as e:
    print(e.strerror, ",文件创建失败")
else:
    print('文件: ', os.path.dirname(sys.argv[0]), '\\devlist.csv 创建成功', sep="")
