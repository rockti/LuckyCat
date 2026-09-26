import mariadb
import logging
import zipfile
import datetime
import Type
from enum import Enum
import os
import configparser

logger = logging.getLogger(__name__)

class Database:
    def __init__(self, *args, **kw):
        config = configparser.ConfigParser()
        config.read('./config.ini')
        user = config['Database']['User']
        pwd = config['Database']['Password']
        db = config['Database']['DB']
        host = config['Database']['Host']
        self.conn = mariadb.connect(user=user, password=pwd, database=db, host=host)
    def backup_database(self):
        cur = self.conn.cursor()
        cur.execute("SHOW TABLES")
        data = ""
        tables = []
        for table in cur.fetchall():
            tables.append(table[0])

        for table in tables:
            data += "DROP TABLE IF EXISTS `" + str(table) + "`;"

            cur.execute("SHOW CREATE TABLE `" + str(table) + "`;")
            data += "\n" + str(cur.fetchone()[1]) + ";\n\n"

            cur.execute("SELECT * FROM `" + str(table) + "`;")
            for row in cur.fetchall():
                data += "INSERT INTO `" + str(table) + "` VALUES("
                first = True
                for field in row:
                    if not first:
                        data += ', '
                    data += '"' + str(field) + '"'
                    first = False
                data += ");\n"
            data += "\n\n"
        now = datetime.datetime.now()
        filename = "./jinchan_backup_" + now.strftime("%Y-%m-%d_%H%M") + ".sql"

        FILE = open(filename,"w")
        FILE.writelines(data)
        FILE.close()
        zipfile.ZipFile(filename+".zip", mode='w',compression=zipfile.ZIP_DEFLATED).write(filename)
        os.remove(filename)
    def to_str(self, value):
        if issubclass(type(value), Enum):
            return value.value
        return str(value)
    def to_insert_sql(self, table_name, obj):
        sql = "insert into " + table_name + " ("
        for i in range(len(obj.__fields__)):
            if i > 0:
                sql += ","
            sql += obj.__fields__[i]
        sql += ") values ("
        dic = Type.asdict(obj)
        for i in range(len(obj.__fields__)):
            if i > 0:
                sql += ","
            sql += "'" + self.to_str(dic[obj.__fields__[i]]) + "'"
        sql += ");"
        return sql
        
    def truncate(self,table):
        try:
            cursor = self.conn.cursor()
            cursor.execute("truncate table {}".format(table))
            cursor.close()
        except mariadb.Error as error:
            print("truncate table {} failed, msg {}".format(table,error))
    def insert(self, table, objs):
        for obj in objs:
            sql = self.to_insert_sql(table,obj)
            cursor = self.conn.cursor()
            try:
                cursor.execute(sql)
            except mariadb.Error as error:
                print("insert table {} failed, sql {} msg {}".format(table,sql,error))
            cursor.close()
    def delete_investors(self):
        self.truncate("t_sync_investorinfo")
    def delete_instruments(self):
        self.truncate("t_sync_instrument")
    def delete_accounts(self):
        self.truncate("t_sync_investoraccount")
    def delete_positions(self):
        self.truncate("t_sync_investorposition")
    def delete_fee(self):
        self.truncate("t_sync_investorfee")
    def delete_margin(self):
        self.truncate("t_sync_investormargin")
    def delete_orders(self):
        self.truncate("t_oper_order")
    def delete_trades(self):
        self.truncate("t_oper_trade")
    def delete_balancechanges(self):
        self.truncate("t_oper_balancechange")
    def delete_positionchanges(self):
        self.truncate("t_oper_positionchange")
    def delete_all(self):
#        self.delete_investors()
        self.delete_instruments()
        self.delete_accounts()
        self.delete_fee()
        self.delete_margin()
        self.delete_orders()
        self.delete_trades()
        self.delete_positions()
        self.delete_positionchanges()
        self.delete_balancechanges()
    def set_tradingday(self,tradingDay):
        sql = "update t_sync_systemstatus set TradingDay = '" + tradingDay +"'"
        cursor = self.conn.cursor()
        try:
            cursor.execute(sql)
        except mariadb.Error as error:
            print("set tradingday failed, sql {} msg {}".format(sql,error))
        cursor.close()


    def insert_investors(self, investors):
        self.insert("t_sync_investorinfo",investors)
    def insert_instruments(self, insts):
        self.insert("t_sync_instrument",insts)
    def insert_accounts(self,accounts):
        accounts = filter(lambda x: x.BrokerID != "", accounts)
        self.insert("t_sync_investoraccount",accounts)
    def insert_positions(self,positions):
        positions = filter(lambda x: x.BrokerID != "", positions)
        self.insert("t_sync_investorposition",positions)
    def insert_fee(self,fees):
        fees = filter(lambda x: x.BrokerID != "", fees)
        self.insert("t_sync_investorfee", fees)
    def insert_margin(self,margins):
        margins = filter(lambda x: x.BrokerID != "", margins)
        self.insert("t_sync_investormargin",margins)
    def insert_all(self, investors,insts, accounts,positions,fees,margins):
        self.insert_investors(investors)
        self.insert_instruments(insts)
        self.insert_accounts(accounts)
        self.insert_positions(positions)
        self.insert_fee(fees)
        self.insert_margin(margins)
