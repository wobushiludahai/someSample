import akshare as ak

stock_zh_index_spot_em_df = ak.stock_zh_index_spot_em(symbol="上证系列指数")
print(stock_zh_index_spot_em_df)

stock_sse_deal_daily_df = ak.stock_sse_deal_daily(date="20240712")
print(stock_sse_deal_daily_df)

# 个股信息
stock_individual_info_em_df = ak.stock_individual_info_em(symbol="000001")
print(stock_individual_info_em_df)

# 行情报价
stock_bid_ask_em_df = ak.stock_bid_ask_em(symbol="000001")
print(stock_bid_ask_em_df)

# 实时行情数据（东财）
# 沪深京A股
stock_zh_a_spot_em_df = ak.stock_zh_a_spot_em()
print(stock_zh_a_spot_em_df)

# 沪A
stock_sh_a_spot_em_df = ak.stock_sh_a_spot_em()
print(stock_sh_a_spot_em_df)

# 深A
stock_sz_a_spot_em_df = ak.stock_sz_a_spot_em()
print(stock_sz_a_spot_em_df)

# 京A
stock_bj_a_spot_em_df = ak.stock_bj_a_spot_em()
print(stock_bj_a_spot_em_df)

# 新股
stock_new_a_spot_em_df = ak.stock_new_a_spot_em()
print(stock_new_a_spot_em_df)

# 创业板
stock_cy_a_spot_em_df = ak.stock_cy_a_spot_em()
print(stock_cy_a_spot_em_df)

# 科创板
stock_kc_a_spot_em_df = ak.stock_kc_a_spot_em()
print(stock_kc_a_spot_em_df)

# 实时行情数据(新浪)
stock_zh_a_spot_df = ak.stock_zh_a_spot()
print(stock_zh_a_spot_df)

# 实时行情数据（雪球）
# stock_individual_spot_xq_df = ak.stock_individual_spot_xq(symbol="SPY")
# print(stock_individual_spot_xq_df)