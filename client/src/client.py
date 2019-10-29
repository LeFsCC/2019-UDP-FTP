
# -*- coding: utf-8 -*-


import sys
from socket import *
import re
import os
import time
from PyQt5 import QtWidgets,QtGui, QtCore, Qt
from PyQt5.QtWidgets import QDesktopWidget, QLabel, QLineEdit, QGridLayout, QTextBrowser, QApplication
from PyQt5.QtWidgets import QFileSystemModel, QTreeView, QWidget, QVBoxLayout
from PyQt5.QtGui import QIcon
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *


MARCO_transfer_file_path = ""
MARCO_server_file_socket = 0


class ftp_client_GUI(QtWidgets.QWidget):
    def __init__(self):
        super(ftp_client_GUI, self).__init__()
        self.initGUI()

    def initGUI(self):
        qr = self.frameGeometry()
        cp = QtWidgets.QDesktopWidget().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())
        self.setWindowTitle("FTP Client")

        #label

        self.title_label = QtWidgets.QLabel(self)
        self.title_label.resize(300, 40)
        self.title_label.move(300, 20)
        self.title_label.setText("FTP  Client  End")
        self.title_label.setStyleSheet(
                          "border-color: rgb(170, 150, 163);"
                          "font: 75 20pt;"
                          "color: rgb(126, 120, 46);")
        # ip_edit

        self.ip_edit = QLineEdit(self)
        self.ip_edit.resize(230, 40)
        self.ip_edit.move(100, 80)
        self.ip_edit.setPlaceholderText("ip like 127.0.0.1")

        #port_edit

        self.port_edit = QLineEdit(self)
        self.port_edit.resize(130, 40)
        self.port_edit.move(370, 80)
        self.port_edit.setPlaceholderText("port like 3000")

        # connect_btn

        self.connect_btn =QtWidgets.QPushButton('connect',self)
        self.connect_btn.resize(150,40)
        self.connect_btn.move(550,80)
        self.connect_btn.clicked.connect(self.on_connect_btn);

        # cmd edit 

        self.sentence_editor = QLineEdit(self)
        self.sentence_editor.resize(400, 40)
        self.sentence_editor.move(100, 130)
        self.sentence_editor.setPlaceholderText("input command and argument, like 'USER anonymous'")

        # cmd btn

        self.cmd_btn =QtWidgets.QPushButton('push command',self)
        self.cmd_btn.resize(150,40)
        self.cmd_btn.move(550,130)
        self.cmd_btn.clicked.connect(self.on_cmd_btn)

        # text browser

        self.text_browser = QTextBrowser(self)
        self.text_browser.resize(450, 160)
        self.text_browser.move(730, 20)

        # file tree

        self.file_model = QFileSystemModel(self);
        dir_path = os.path.dirname(os.path.abspath(__file__))

        self.file_model.setRootPath(dir_path);
        self.tree = QTreeView(self);
        self.tree.setModel(self.file_model);
        self.tree.setRootIndex(self.file_model.index(dir_path))

        self.tree.setAnimated(True);
        self.tree.setIndentation(20);
        self.tree.setSortingEnabled(True)
        self.tree.setWindowTitle("Dir View")
        self.tree.resize(500, 370);
        self.tree.move(830,200);

        # timer

        self.timer = QBasicTimer()
        self.timer.start(1000, self)
        


        # internet speed display
        self.width = 200;
        self.height = 20;
        self.bias = 1200;

        self.cur_load_volume = 0;
        self.pre_load_volume = 0;

        self.internet_speed_label = QLabel(self)
        self.internet_speed_label.move(110, 600)
        self.internet_speed_label.resize(300, 20)
        self.internet_speed_label.setStyleSheet(
                  "border-color: rgb(170, 150, 163);"
                  "font: 30 12pt;"
                  "color: rgb(126, 120, 46);");


        # file total load

        self.file_total_upload = 0;
        self.file_total_download = 0;
        self.file_total_upload_num = 0;
        self.file_total_download_num = 0;

        self.file_total_upload_label = QLabel(self);
        self.file_total_upload_num_label = QLabel(self);
        self.file_total_download_label = QLabel(self);
        self.file_total_download_num_label = QLabel(self);

        self.file_total_upload_label.move(self.bias, self.height + 40);
        self.file_total_upload_num_label.move(self.bias, self.height + 60);
        self.file_total_download_label.move(self.bias, self.height + 80);
        self.file_total_download_num_label.move(self.bias, self.height + 100);

        self.file_total_upload_label.resize(self.width, self.height);
        self.file_total_upload_num_label.resize(self.width, self.height);
        self.file_total_download_label.resize(self.width, self.height);
        self.file_total_download_num_label.resize(self.width, self.height);

        self.file_total_upload_label.setStyleSheet(
                  "border-color: rgb(170, 150, 163);"
                  "font: 30 12pt;"
                  "color: rgb(126, 120, 46);");

        self.file_total_upload_num_label.setStyleSheet(
                  "border-color: rgb(170, 150, 163);"
                  "font: 30 12pt;"
                  "color: rgb(126, 120, 46);");

        self.file_total_download_label.setStyleSheet(
                  "border-color: rgb(170, 150, 163);"
                  "font: 30 12pt;"
                  "color: rgb(126, 120, 46);");

        self.file_total_download_num_label.setStyleSheet(
                  "border-color: rgb(170, 150, 163);"
                  "font: 30 12pt;"
                  "color: rgb(126, 120, 46);");

        # tabel widget

        self.TableWidget = QTableWidget(self)
        self.TableWidget.resize(710, 370);
        self.TableWidget.move(100, 200);
        self.TableWidget.setRowCount(0);
        self.TableWidget.setColumnCount(3);
        self.TableWidget.setHorizontalHeaderLabels(['name','info','code'])
        self.TableWidget.setColumnWidth(0, 80);
        self.TableWidget.setColumnWidth(1, 560);
        self.TableWidget.setColumnWidth(2, 65);

        # progress bar
        self.step = 0;
        self.retr_file_size = 0;
        self.pbar = QProgressBar(self)
        self.pbar.resize(700, 15);
        self.pbar.move(110, 580);
        self.pbar.setValue(self.step);


        self.resize(1400,650);
        self.show();

        self.client_command_socket = 1;

        self.client_file_socket = 1;
        self.server_file_socket = 1;

        self.file_port = 3000;
        self.file_addr = "127.0.0.1";

        self.server_port = 3000;
        self.server_addr = "127.0.0.1"
        self.reply_sentence = "";
        self.req_sentence = "";

        self.client_command = "";
        self.client_argument = "";
        self.client_state = 0;

        self.offset_read = 0


        self.transfer_file_path = "";


        self.common_command = {"USER", "PASS", "TYPE", "MKD", "RNFR", "RMD", "RNTO", "PWD", "CWD", "SYST", "REST"};
        self.parti_command = {"QUIT", "PORT", "PASV", "RETR", "LIST", "STOR"};


    def timerEvent(self, *args, **kwargs):
        volume = self.cur_load_volume - self.pre_load_volume;
        byte_volume = volume % 1024;
        kbyte_volume = volume / 1024;
        
        self.internet_speed_label.setText("load file rate : " + str(kbyte_volume) + " kb/s");

        self.pre_load_volume = self.cur_load_volume;

        self.file_total_upload_label.setText("upload : " + str(int(self.file_total_upload / 1024)) + " kb")
        self.file_total_upload_num_label.setText("upload : " + str(int(self.file_total_upload_num)) + " files")
        self.file_total_download_label.setText("download : " + str(int(self.file_total_download / 1024)) + " kb")
        self.file_total_download_num_label.setText("download : " + str(self.file_total_download_num) + " files")
        if self.step == 100:
            self.step = 0;
            self.pbar.setValue(0)
        return;

    # send client connect msg
    def on_connect_btn(self):

        port  = self.port_edit.text();
        ip = self.ip_edit.text();
        try:
            self.client_command_socket = socket(AF_INET, SOCK_STREAM);
            self.client_command_socket.connect((ip, int(port)));
            self.reply_sentence = str(self.client_command_socket.recv(4096).decode())
            self.add_item("server " + str(self.reply_sentence));
            self.client_state = 1;
        except:
            self.client_state = 1;
            sent = "system 0 already connect!";
            self.add_item(sent);

     # receive client input  and push
    def on_cmd_btn(self):
        sentence = self.sentence_editor.text();
        cmd_arg = sentence.split(' ');
        cmd_arg[0] = cmd_arg[0].upper(); # according to rfc959
        temp = ""
        if self.client_state == 0:
            sent = "system 0 not connect!";
            self.add_item(sent);

        elif self.client_state == 1 or self.client_state == 2 or self.client_state == 3:

            if cmd_arg[0] in self.common_command:

                if cmd_arg[0] == "REST":
                     sentence = sentence + " " + str(self.offset_read)

                sentence = sentence + "\r\n"
                self.client_command_socket.send(sentence.encode())
                senten = self.client_command_socket.recv(4096)

                self.reply_sentence = senten.decode()
                self.add_item("server " + self.reply_sentence);


            elif cmd_arg[0] in self.parti_command:

                if cmd_arg[0] == "QUIT":
                    sentence = sentence + "\r\n"
                    self.client_command_socket.send(sentence.encode())
                    self.reply_sentence = str(self.client_command_socket.recv(4096).decode())
                    self.add_item("server " + str(self.reply_sentence));
                    self.client_command_socket.close()
                    return;
                elif cmd_arg[0] == "PORT":
                    port_addr = cmd_arg[1].split(',');
                    if len(port_addr) < 6:
                       self.text_browser.append("system : invalid PORT addr!\r\n");
                       self.text_browser.moveCursor(self.text_browser.textCursor().End);
                       return;
                    else:
                       addr = port_addr[0] + '.' + port_addr[1] + '.' + port_addr[2] + '.' + port_addr[3];
                       port = int(port_addr[4]) *256 + int(port_addr[5]);
                       self.client_file_socket = socket(AF_INET,SOCK_STREAM);
                       self.client_file_socket.bind((addr, port));
                       self.client_file_socket.listen(10);
                       sentence = sentence + "\r\n"
                       self.client_command_socket.send(sentence.encode());
                       self.reply_sentence = str(self.client_command_socket.recv(4096).decode())
                       self.add_item("server " + str(self.reply_sentence));
                       self.client_state = 2;
                elif cmd_arg[0] == "PASV":
                       sentence = sentence + "\r\n"
                       self.client_command_socket.send(sentence.encode());
                       self.reply_sentence = str(self.client_command_socket.recv(4096).decode())
                       self.add_item("server " + self.reply_sentence);
                       li = 1;
                       li = re.search(r"\(.*\)", self.reply_sentence);
                       
                       addr = li.group()[1:-1];
                       addr = addr.split(',');
                       port = int(addr[-2]) * 256 + int(addr[-1])
                       addr = addr[0] + '.' + addr[1] + '.' + addr[2] + '.' + addr[3];
                       self.file_addr = addr[0:];
                       self.file_port = port;
                       self.client_state = 3;
                       self.client_file_socket = socket(AF_INET,SOCK_STREAM);
                elif cmd_arg[0] == "RETR":
                    

                    if self.client_state == 2:
                       sentence = sentence + "\r\n"
                       self.client_command_socket.send(sentence.encode());
                       self.server_file_socket, temp = self.client_file_socket.accept();

                       self.reply_sentence = str(self.client_command_socket.recv(4096).decode())
                       
                       self.add_item("server " + str(self.reply_sentence));

                       if self.reply_sentence.split(' ')[0] != "550" and self.reply_sentence.split(' ')[0] != "503":
                          self.retr_file_size = self.get_retr_file_size(self.reply_sentence)
                          self.transfer_file_path = cmd_arg[1][:]
                          self.recv_data();

                       self.server_file_socket.close();
                       self.client_file_socket.close();
                       self.server_file_socket = -1;
                       self.client_file_socket = -1;
                    elif self.client_state == 3:
                       sentence = sentence + "\r\n"
                       self.client_command_socket.send(sentence.encode());
                       self.client_file_socket.connect((self.file_addr, self.file_port))
                       self.reply_sentence = str(self.client_command_socket.recv(4096).decode())
                       self.add_item("server " + str(self.reply_sentence));
                       self.server_file_socket = self.client_file_socket
                       if self.reply_sentence.split(' ')[0] != "550" and self.reply_sentence.split(' ')[0] != "503": 
                          self.retr_file_size = self.get_retr_file_size(self.reply_sentence)
                          self.transfer_file_path = cmd_arg[1][:]
                          self.recv_data();

                       self.server_file_socket.close();
                    else:
                       self.add_item("system 0 require PASV/PORT mode")

                    if self.reply_sentence.split(' ')[0] != "550" and len(self.reply_sentence.split("\r\n")[1]) == 0 and self.step == 100 and self.reply_sentence.split(' ')[0] != "503"  and self.client_state != 1:
                        self.reply_sentence = str(self.client_command_socket.recv(4096).decode())
                        self.add_item("server " + str(self.reply_sentence));
                    self.client_state = 1;


                elif cmd_arg[0] == "LIST":
                    
                    if self.client_state == 2:
                       sentence = sentence + "\r\n"
                       self.client_command_socket.send(sentence.encode());
                       self.server_file_socket, temp = self.client_file_socket.accept();
                       self.reply_sentence = str(self.client_command_socket.recv(4096).decode())
                       self.add_item("server " + str(self.reply_sentence));

                       if self.reply_sentence.startswith("150"):
                           self.print_list(self.server_file_socket)

                       self.server_file_socket.close();
                       self.client_file_socket.close();
                       self.server_file_socket = -1;
                       self.client_file_socket = -1;
                    elif self.client_state == 3:
                       sentence = sentence + "\r\n"
                       self.client_command_socket.send(sentence.encode());
                       self.client_file_socket.connect((self.file_addr, self.file_port))

                       self.reply_sentence = str(self.client_command_socket.recv(4096).decode())
                       self.add_item("server " + str(self.reply_sentence));
                       if self.reply_sentence.startswith("150"):
                           self.server_file_socket = self.client_file_socket
                           self.print_list(self.server_file_socket)

                       self.server_file_socket.close();
                    else:
                       self.add_item("system 0 require PASV/PORT mode")

                    if self.reply_sentence.startswith("150") and self.client_state != 1:
                        self.reply_sentence = str(self.client_command_socket.recv(4096).decode())
                        self.add_item("server " + str(self.reply_sentence));
                        self.client_state = 1;
                elif cmd_arg[0] == "STOR":

                    if self.get_file_size(cmd_arg[1]) == 0:
                      self.add_item("system 0 file not found");
                      return
                    
                    if self.client_state == 2:
                       sentence = sentence + "\r\n"
                       self.client_command_socket.send(sentence.encode());
                       self.server_file_socket, temp = self.client_file_socket.accept();

                       self.reply_sentence = str(self.client_command_socket.recv(4096).decode())
                       self.add_item("server " + str(self.reply_sentence));

                       self.send_data(cmd_arg[1], self.server_file_socket);
                       self.server_file_socket.close();
                       self.client_file_socket.close();
                       self.server_file_socket = -1;
                       self.client_file_socket = -1;
                    elif self.client_state == 3:
                       sentence = sentence + "\r\n"
                       self.client_command_socket.send(sentence.encode());
                       self.client_file_socket.connect((self.file_addr, self.file_port))

                       self.reply_sentence = str(self.client_command_socket.recv(4096).decode())
                       self.add_item("server " + str(self.reply_sentence));

                       self.server_file_socket = self.client_file_socket
                       self.send_data(cmd_arg[1], self.server_file_socket);
                       self.server_file_socket.close();
                    else:
                       self.add_item("system 0 require PASV/PORT mode")
                    if self.client_state != 1:
                       self.reply_sentence = str(self.client_command_socket.recv(4096).decode())
                       self.add_item("server " + str(self.reply_sentence));
                    self.client_state = 1;
                else:
                    self.text_browser.append("system : invalid command");
                    self.text_browser.moveCursor(self.text_browser.textCursor().End);

            else:
                sent = "system 0 not connect!";
                self.add_item(sent);

    def recv_data(self):

        file_path = self.transfer_file_path[:]
        sock = self.server_file_socket

        if self.offset_read == 0:
             fp = open(file_path, "wb");
        else:
             fp = open(file_path, "ab");
             fp.seek(self.offset_read-1, 0)

        self.retr_file_size = self.retr_file_size - self.offset_read


        data_stream = sock.recv(4096)
        self.pre_load_volume = 0;
        self.cur_load_volume = 0;

        count = 0;
        time1 = int(time.time() * 10000);
        time2 = int(time.time() * 10000);

        while data_stream:
            self.cur_load_volume += len(data_stream);
            self.step = (self.cur_load_volume / self.retr_file_size) * 100;
            self.pbar.setValue(self.step);
            fp.write(data_stream);
            data_stream = '';
            data_stream = sock.recv(4096);

            count += 1;
            if count == 20:
                volume = self.cur_load_volume - self.pre_load_volume;
                byte_volume = volume % 1024;
                kbyte_volume = volume / 1024;
                time2 = int(time.time() * 10000);
                interval = (time2 - time1) / 10000;

                if interval != 0:
                    self.internet_speed_label.setText("load file rate : " + str(round(kbyte_volume / interval, 1)) + " kb/s");

                self.pre_load_volume = self.cur_load_volume
                time1 = time2;
                count = 0;
            else:
                continue;


        if self.step == 100:
            self.offset_read = 0;
        else:
            self.offset_read = self.cur_load_volume;




        fp.close();
        self.file_total_download += self.cur_load_volume;
        self.file_total_download_num += 1;
        self.pre_load_volume = 0;
        self.cur_load_volume = 0;
        return;

    def send_data(self, file_path, sock):
        fp = open(file_path, "rb");
        n = 2;
        buf = ""
        
        file_size = int(self.get_file_size(file_path));

        self.pre_load_volume = 0;
        self.cur_load_volume = 0;
        count = 0;
        time1 = int(time.time() * 10000);
        time2 = int(time.time() * 10000);

        while n > 0:
            buf = fp.read(4096);
            n = len(buf);
            self.cur_load_volume += n;
            self.step = (self.cur_load_volume / file_size) * 100;
            self.pbar.setValue(self.step);
            count += 1;
            if count == 20:
                volume = self.cur_load_volume - self.pre_load_volume;
                byte_volume = volume % 1024;
                kbyte_volume = volume / 1024;
                time2 = int(time.time() * 10000);
                interval = (time2 - time1) / 10000;
                if interval != 0:
                    self.internet_speed_label.setText("load file rate : " + str(round(kbyte_volume / interval, 1)) + " kb/s");

                self.pre_load_volume = self.cur_load_volume
                time1 = time2;
                count = 0;
            sock.send(buf);

        fp.close();
        self.file_total_upload += self.cur_load_volume;
        self.file_total_upload_num += 1;
        self.pre_load_volume = 0;
        self.cur_load_volume = 0;
        return;
    def print_list(self, sock):
        n = 2
        data_stream = sock.recv(4096)
        while n > 0:
            self.text_browser.setText(data_stream.decode());
            self.text_browser.moveCursor(self.text_browser.textCursor().End);
            data_stream = sock.recv(4096)
            n = len(data_stream)
        return;

    def recv_info(self):
        self.reply_sentence = self.client_command_socket.recv(4096).decode();
        self.text_browser.append("server : " + self.reply_sentence);
        self.text_browser.moveCursor(self.text_browser.textCursor().End);
        return;

    def add_item(self,sentence):
        spl = sentence.split(' ')
        name = spl[0];
        code = spl[1];
        info = ""
        for i in spl[2:]:
            info += i + ' ';

        self.TableWidget.setRowCount(self.TableWidget.rowCount() + 1);
        newItem=QTableWidgetItem(name)
        self.TableWidget.setItem(self.TableWidget.rowCount()-1, 0, newItem)
        newItem=QTableWidgetItem(info)
        self.TableWidget.setItem(self.TableWidget.rowCount()-1, 1, newItem)
        newItem=QTableWidgetItem(code)
        self.TableWidget.setItem(self.TableWidget.rowCount()-1, 2, newItem)

    def get_file_size(self, file_path):
        try:
            size = os.path.getsize(file_path)
            return size
        except Exception as err:
            return 0;

    def get_retr_file_size(self,sentence):
        li = re.search(r"\(.*\)", self.reply_sentence);
        if li.group() != None:
            temp = li.group()[1:-1];
            cv = re.search(r"\d+", temp)
            return int(cv.group());
        else:
            return 99999999;


if __name__ == '__main__':
    
    app = QtWidgets.QApplication(sys.argv)
    gui = ftp_client_GUI()
    sys.exit(app.exec_())
