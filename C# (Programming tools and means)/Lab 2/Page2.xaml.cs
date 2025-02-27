using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

namespace Lab_1;

public partial class Page2 : ContentPage
{
    bool lastStartbutton = false;
	public Page2()
	{
		InitializeComponent();
    }
    private async void StartClicked(object sender, EventArgs e)
	{
        lastStartbutton = true;
        Token?.Cancel();
        Token = new CancellationTokenSource();

        try
        {
            label.Text = "Calculating...";
            FindInt = Task.Run(() => GetIntegral(Token.Token));
            double result = await FindInt;

            label.Text = $"Result = {result}";
            progressBar.Progress = 1;
            progressLabel.Text = $"100 %";
        }
        catch(OperationCanceledException)
        {
            if(!lastStartbutton)
                label.Text = "Task canceled";
        }
    }
    private void CancelClicked(object sender, EventArgs e)
    {
        lastStartbutton = false;
        Token?.Cancel();
    }
    private CancellationTokenSource Token;
    private Task<double> FindInt;
    private async Task<double> GetIntegral(CancellationToken Token)
    {
        
        double step = 0.005, ans = 0;
        int last = -1;
        int upd_step = 0;
        for (double i = 0; i <= 1; i += step,upd_step++)
        {
            Token.ThrowIfCancellationRequested(); //if cancel pressed button 
            ans += Math.Sin(i) * step;

            if (upd_step==5 && last != (int)(i * 100))
            {
                
                last = (int)(i * 100);
                Device.BeginInvokeOnMainThread(() =>
                {
                    progressBar.Progress = (double)last / (double)100;
                    progressLabel.Text = $"{last} %";
                    
                }
                );
                upd_step = 0;
            }
           
            //wait(10);
            await Task.Delay(1, Token);// check cancel
        }
        
        return ans;
    }

    void wait(int x)
    {
        int a;
        for (int i = 0; i < x; i++)
            a = 2 * 2;
    }
}