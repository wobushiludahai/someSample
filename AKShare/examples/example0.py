
import akshare as ak

# 获取上海证券交易所总貌
stock_sse_summery_df = ak.stock_sse_summary()
print(stock_sse_summery_df)

print("\n获取每日概览:\n")
stock_sse_deal_daily = ak.stock_sse_deal_daily(date="20240726")
print(stock_sse_deal_daily)

