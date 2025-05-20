namespace Lab_1
{
    public partial class MainPage : ContentPage
    {
        String input="";
        public MainPage()
        {
            InitializeComponent();
        }

        private void PressButton(object sender, EventArgs e)
        {
            if (sender is Button button)
            {

                if(button.Text=="exp(x)")
                {
                    double res = 0;
                    bool no_exc = true;
                    if(double.TryParse(input,out res))
                    {
                        try
                        {
                            res=Math.Exp(res);
                        }
                        catch(Exception ex)
                        {
                            no_exc = false;
                        }

                        if (no_exc)
                        {
                            input="";
                            DisplayLabel.Text = res.ToString();
                            return;
                        }
                          
                    }

                    DisplayLabel.Text = "Error";
                    input = "";
                    return;
                }

                if(button.Text== "C")
                {
                    if(input.Length>0)
                    input = input.Remove(input.Length - 1);
                }
                else
                    input += button.Text;

                DisplayLabel.Text = input;
                
            }
        }
    }

}
