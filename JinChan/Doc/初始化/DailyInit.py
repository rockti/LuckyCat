import xlrd
import mariadb
import wx
import datetime
import wx.adv
import wx.grid
import logging
import zipfile
import Type
import Jinchan
from enum import Enum
from collections import defaultdict

logger = logging.getLogger(__name__)

class WxTextCtrlHandler(logging.Handler):
    def __init__(self, ctrl):
        logging.Handler.__init__(self)
        self.ctrl = ctrl

    def emit(self, record):
        s = self.format(record) + '\n'
        wx.CallAfter(self.ctrl.WriteText, s)

class MySheet(wx.grid.Grid):

    def __init__(self, *args, **kw):
        super(MySheet, self).__init__(*args, **kw)

        self.InitUI()

    def InitUI(self):

        nOfRows = 55
        nOfCols = 25

        self.row = self.col = 0
        self.CreateGrid(nOfRows, nOfCols)

        self.SetColLabelSize(20)
        self.SetRowLabelSize(50)

        self.Bind(wx.grid.EVT_GRID_SELECT_CELL, self.OnGridSelectCell)

        for i in range(nOfRows):
            self.SetRowSize(i, 20)

        for i in range(nOfCols):
            self.SetColSize(i, 75)

    def OnGridSelectCell(self, e):

        self.row, self.col = e.GetRow(), e.GetCol()

        control = self.GetParent().GetParent().position
        value =  self.GetColLabelValue(self.col) + self.GetRowLabelValue(self.row)
        control.SetValue(value)

        e.Skip()
class InitFrame(wx.Frame):

    def __init__(self, *args, **kw):
        super(InitFrame, self).__init__(*args, **kw)
        self.ep = Type.ExcelProcessor()

        self.InitUI()
    def add_file_component(self, pnl,vbox,variable,name,validate_func):
        
        button = wx.Button(pnl, label="选择{}文件".format(name), pos=(360, 80))
        button.Bind(wx.EVT_BUTTON, self.generate_file_on_click(variable,name,validate_func))
        label = wx.StaticText(pnl)
        setattr(self,variable+"FileLabel", label)
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        hbox.Add(button,flag=wx.RIGHT|wx.Center, border=8)
        hbox.Add(label,flag=wx.RIGHT|wx.Center, border=16)
        vbox.Add(hbox,flag=wx.RIGHT|wx.Center, border=16)

    def generate_file_on_click(self, variable, name,validate_func):
        def F(e):
            with wx.FileDialog(self, "打开{}文件".format(name), wildcard="Excel files (*.xls)|*.xls",
                        style=wx.FD_OPEN | wx.FD_FILE_MUST_EXIST) as fileDlg:

                if fileDlg.ShowModal() == wx.ID_CANCEL:
                    return     
                file = fileDlg.GetPath()
                setattr(self,variable+'File',file)
                if not validate_func(file):
                    wx.MessageBox('{}文件格式不对,请重新选择'.format(name), '错误', wx.OK | wx.ICON_INFORMATION)
                    return
                getattr(self,variable + 'FileLabel').SetLabel(file)

        return F

    def InitUIOfInit(self, parent):
        pnl = wx.Panel(parent)
        vbox = wx.BoxSizer(wx.VERTICAL)

        labelTradingDay = wx.StaticText(pnl, pos=(10,80),label="设置交易日期")
        tradingDayPicker = wx.adv.GenericDatePickerCtrl(pnl, pos=(180,80),size=(130,-1),
                                       style = wx.TAB_TRAVERSAL
                                       | wx.adv.DP_DROPDOWN
                                       | wx.adv.DP_SHOWCENTURY )
        self.Bind(wx.adv.EVT_DATE_CHANGED, self.OnDateChanged, tradingDayPicker)

        hboxTradingDay = wx.BoxSizer(wx.HORIZONTAL)
        hboxTradingDay.Add(labelTradingDay,flag=wx.RIGHT|wx.Center, border=8)
        hboxTradingDay.Add(tradingDayPicker,flag=wx.RIGHT|wx.Center, border=16)
        vbox.Add(hboxTradingDay,flag=wx.RIGHT|wx.Center, border=16)


        self.add_file_component(pnl,vbox,"investor",'客户信息',self.ep.validate_investor_file)
        self.add_file_component(pnl,vbox,"account",'客户资金',self.ep.validate_account_file)
        self.add_file_component(pnl,vbox,"deferPosition",'延期持仓',self.ep.validate_defer_position_file)
        self.add_file_component(pnl,vbox,"spotPosition",'现货持仓',self.ep.validate_spot_position_file)
        self.add_file_component(pnl,vbox,"deferInst",'延期品种',self.ep.validate_defer_inst_file)
        self.add_file_component(pnl,vbox,"spotInst",'现货品种',self.ep.validate_spot_inst_file)
        self.add_file_component(pnl,vbox,"margin",'保证金率',self.ep.validate_margin_file)
        self.add_file_component(pnl,vbox,"fee",'手续费率',self.ep.validate_fee_file)

        initButton = wx.Button(pnl, label='开始初始化', size=(100,40))
        initButton.Bind(wx.EVT_BUTTON, self.OnInitButton)
        vbox.Add((-1, 25))
        hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        hbox1.Add(initButton)
        vbox.Add(hbox1,flag=wx.ALIGN_RIGHT|wx.RIGHT, border=10)

        hbox3 = wx.BoxSizer(wx.HORIZONTAL)
        self.logWindow = wx.TextCtrl(pnl, style=wx.TE_MULTILINE|wx.TE_READONLY|wx.HSCROLL)
        hbox3.Add(self.logWindow, proportion=1, flag=wx.EXPAND)
        vbox.Add(hbox3, proportion=1, flag=wx.LEFT|wx.RIGHT|wx.EXPAND, border=10)
        handler = WxTextCtrlHandler(self.logWindow)
        logger.addHandler(handler)
        FORMAT = "%(asctime)s %(levelname)s %(message)s"
        handler.setFormatter(logging.Formatter(FORMAT))
        logger.setLevel(logging.DEBUG)

        pnl.SetSizer(vbox)
        return pnl

    def InitUI(self):
        self.root = wx.Panel(self)
        self.nb = wx.Notebook(self.root)
        initFrame = self.InitUIOfInit(self.nb)

        investorPanel = wx.Panel(self.nb)
        grid = MySheet(investorPanel)
        gridBox = wx.BoxSizer(wx.VERTICAL)
        gridBox.Add(grid, 1, wx.EXPAND)
        investorPanel.SetSizer(gridBox)
        self.nb.AddPage(initFrame, "初始化")
        self.nb.AddPage(investorPanel,"客户")

        sizer = wx.BoxSizer()
        sizer.Add(self.nb, 1, wx.EXPAND)
        self.root.SetSizer(sizer)
        self.SetSize((1024, 768))
        self.SetTitle("金蟾初始化程序 2.2.1.0")
        self.Centre()
        logger.log(logging.INFO,"程序启动...")

    def OnDateChanged(self, evt):
        sel_date = evt.GetDate()
        self.tradingDay = sel_date.Format("%Y%m%d")

    def toInvestorBrokerMap(self, investors):
        investorMap = defaultdict(lambda : "")
        for v in investors:
            investorMap[v.InvestorID] = v.BrokerID
        return investorMap
    def brokers(self,investors):
        brokers = []
        for v in investors:
            brokers.append(v.BrokerID)
        return brokers
    def OnInitButton(self,evt):
        if not hasattr(self, "tradingDay"):
            wx.MessageBox("交易日期还未选择，请选择",'提醒', wx.OK | wx.ICON_INFORMATION)
            return
        if not self.ep.all_file_selected():
            wx.MessageBox("还有初始化文件没有选择，请选择",'提醒', wx.OK | wx.ICON_INFORMATION)
            return
        wx.MessageBox('开始初始化，初始化日志为{}'.format(self.tradingDay), '确认', wx.YES_NO | wx.ICON_INFORMATION)
        logger.log(logging.INFO,"开始初始化，初始化日期为{}".format(self.tradingDay))
        
        logger.log(logging.INFO,"开始备份数据库。。。")
        db = Jinchan.Database()
        db.backup_database()
        logger.log(logging.INFO,"备份数据库完成。。。")

        logger.log(logging.INFO,"开始删除旧数据。。。")
        db.delete_all()
        logger.log(logging.INFO,"删除旧数据完成。。。")

        logger.log(logging.INFO,"开始设置交易日期。。。")
        db.set_tradingday(self.tradingDay)
        logger.log(logging.INFO,"设置交易日期完成。。。")

        logger.log(logging.INFO, "开始从文件加载数据")
        investors = self.ep.load_investors()
        investorBrokers = self.toInvestorBrokerMap(investors)
        brokers = self.brokers(investors)
        insts = self.ep.load_insts()
        accounts = self.ep.load_accounts(investorBrokers)
        fees = self.ep.load_fees(investorBrokers)
        margins = self.ep.load_margin(investorBrokers)
        positions = self.ep.load_positions(insts,investorBrokers)
        db.insert_all(investors,insts,accounts,positions,fees,margins)
        logger.log(logging.INFO, "加载完成")

        logger.log(logging.INFO, "初始化结束")


app = wx.App()

frame = InitFrame(None)
frame.Show()
app.MainLoop()