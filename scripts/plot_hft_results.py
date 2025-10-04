import pandas as pd
import matplotlib.pyplot as plt

def plot_csv(filename, x_col, y_cols, title, xlabel, ylabel, output_png):
    df = pd.read_csv(filename)
    print(f"\n{filename} columns: {list(df.columns)}")
    if x_col not in df.columns:
        print(f"Error: X column \"{x_col}\" not found in {filename}. Skipping plot.")
        return
    plt.figure(figsize=(10, 6))
    for col in y_cols:
        if col in df.columns:
            plt.plot(df[x_col], df[col], label=col)
        else:
            print(f"Warning: Y column \"{col}\" not found in {filename}.")
    plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(output_png)
    print(f"Plot saved as {output_png}")
    plt.close()

def main():
    files_and_plots = [
        ('dashboard_timeseries.csv', 'Time', ['Price', 'MovingAvg', 'Volume', 'CumPnL'], 'dashboard_metrics.png', 'Trading Dashboard Metrics', 'Time', 'Value'),
        ('price_series.csv', 'Time', ['Price'], 'price_over_time.png', 'Price Over Time', 'Time', 'Price'),
        ('Synthetic_Price_data.csv', 'Time', ['Price'], 'synthetic_price.png', 'Synthetic Price', 'Time', 'Price'),
        ('Cumulative_PnL_data.csv', 'Time', ['PnL'], 'cumulative_pnl.png', 'Cumulative P&L Over Time', 'Time', 'P&L'),
        ('Moving_Average_data.csv', 'Time', ['MovingAvg'], 'moving_average.png', 'Moving Average Over Time', 'Time', 'Moving Average')
    ]
    for fname, xcol, ycols, opng, title, xlabel, ylabel in files_and_plots:
        plot_csv(fname, xcol, ycols, title, xlabel, ylabel, opng)

if __name__ == '__main__':
    main()
