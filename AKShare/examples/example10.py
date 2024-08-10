import akshare as ak
import pandas as pd

# 获取个股基本面

# 设置 pandas 选项以显示所有行和列
# pd.set_option('display.max_rows', None)  # 显示所有行
# pd.set_option('display.max_columns', None)  # 显示所有列


# 关键指标
# 新浪
stock_financial_abstract_df = ak.stock_financial_abstract(symbol="600004")
print(stock_financial_abstract_df)

# 同花顺
# indicator="按报告期"; choice of {"按报告期", "按年度", "按单季度"}
stock_financial_abstract_ths_df = ak.stock_financial_abstract_ths(symbol="000063", indicator="按报告期")
print(stock_financial_abstract_ths_df)

# 财务指标
stock_financial_analysis_indicator_df = ak.stock_financial_analysis_indicator(symbol="600004", start_year="2020")
print(stock_financial_analysis_indicator_df)

# 历史分红
stock_history_dividend_df = ak.stock_history_dividend()
print(stock_history_dividend_df)

# 十大流通股东
stock_gdfx_free_top_10_em_df = ak.stock_gdfx_free_top_10_em(symbol="sh688686", date="20210630")
print(stock_gdfx_free_top_10_em_df)

# 十大股东
stock_gdfx_top_10_em_df = ak.stock_gdfx_top_10_em(symbol="sh688686", date="20210630")
print(stock_gdfx_top_10_em_df)

# 主要股东
stock_main_stock_holder_df = ak.stock_main_stock_holder(stock="600004")
print(stock_main_stock_holder_df)

# 获取个股指标：如市盈率，市净率等
stock_a_indicator_lg_df = ak.stock_a_indicator_lg(symbol="000001")
print(stock_a_indicator_lg_df)

# 获取主板市盈率
# symbol="上证"; choice of {"上证", "深证", "创业板", "科创版"}
stock_market_pe_lg_df = ak.stock_market_pe_lg(symbol="上证")
print(stock_market_pe_lg_df)