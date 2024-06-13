import akshare as ak
import mplfinance as mpf  # Please install mplfinance as follows: pip install mplfinance

from Toolset import *

stock_szse_summary_df = ak.stock_szse_summary(date=get_today_date())
print(stock_szse_summary_df)