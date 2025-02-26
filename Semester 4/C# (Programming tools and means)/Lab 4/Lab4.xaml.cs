using Lab_1.Entities;
using Lab_1.Services;
using System.Collections.ObjectModel;

namespace Lab_1;

public partial class Lab4 : ContentPage
{
	public IRateService rateService;
	ObservableCollection<CurrencyModel>_currencies = new ObservableCollection<CurrencyModel>();
    ObservableCollection<CurrencyModel> _fullCurs = new ObservableCollection<CurrencyModel>();
    List<string> Need;
	public Lab4(IRateService rateService)
	{
		InitializeComponent();
		this.rateService = rateService;
        datePicker.Date = DateTime.Now; 
        datePicker_DateSelected(this, null);

        Need = new List<string>
        {
            new string("USD"),
            new string("EUR"),
            new string("CHF"),
            new string("GBP"),
            new string("CNY"),
            new string("RUB")
        };

        LeftNumb.IsEnabled = false;
        RightNumb.IsEnabled = false;
    }

    private async void datePicker_DateSelected(object sender, DateChangedEventArgs e)//Upd cur
    {
       _currencies?.Clear();
       _fullCurs?.Clear();
        //get
       var cur = await rateService.GetRatesAsync(datePicker.Date);


        foreach (var currency in cur)
        {
            if (Need.Contains(currency.Cur_Abbreviation))
            {
                _currencies.Add(new CurrencyModel {
                                                    Name = currency.Cur_Abbreviation,
                                                    Currency = currency.Cur_OfficialRate,
                                                    Scale = currency.Cur_Scale,
                                                   });
                
                _fullCurs.Add(new CurrencyModel
                {
                    FullName = currency.Cur_Name,
                });
            }
            
        }
        
        if (_fullCurs.Count > 0)
        {

            _fullCurs.Add(new CurrencyModel
            {
                FullName = "Белорусский рубль",
            });

            label.Text = "Choose currency";
            label.TextColor = Colors.Blue;
            Left.IsVisible = true;
            Right.IsVisible = true;
            RightNumb.IsVisible = true;
            LeftNumb.IsVisible = true;
            RightNumb.IsEnabled = false;
            LeftNumb.IsEnabled = false;
        }
        else
        {
            label.Text = "Can't get info on chosen date";
            label.TextColor = Colors.Red;
            Left.IsVisible = false;
            Right.IsVisible = false;
            RightNumb.IsVisible = false;
            LeftNumb.IsVisible = false;
            
        }
        CurrencyView.ItemsSource = _currencies;
        Left.ItemsSource = _fullCurs;
        Right.ItemsSource = _fullCurs;

        //clear converter data

    }

    private void Left_SelectedIndexChanged(object sender, EventArgs e)
    {
        if (AutoChange)
        { 
            AutoChange = false;
            return; 
        }

        if (Left.SelectedIndex != 6) //!BYN
        {
            AutoChange = true;
            Right.SelectedIndex = 6;
            AutoChange = false;
        }
        else if (Right.SelectedIndex == -1)
        {
            AutoChange = true;
            Right.SelectedIndex = 1;
            AutoChange = false;
            return;
        }

        if (Left.SelectedIndex == Right.SelectedIndex)
        {
            AutoChange = true;
            if (Right.SelectedIndex != 6)
            {
                Right.SelectedIndex = 6;
            }
            else
                Right.SelectedIndex = 1;
            AutoChange = false;
        }
      

        AutoChange = true;
        LeftNumb.Text = "";
        AutoChange = true;
        RightNumb.Text = "";
        AutoChange = false;
        LeftNumb.IsEnabled = true;
        RightNumb.IsEnabled = true;
    }
    private void Right_SelectedIndexChanged(object sender, EventArgs e)
    {
        if (AutoChange)
        {
            AutoChange = false;
            return;
        }

        if (Right.SelectedIndex != 6) //!BYN
        {
            AutoChange = true;
            Left.SelectedIndex = 6;
            AutoChange = false;
        }
        else if(Left.SelectedIndex == -1)
        {
            AutoChange = true;
            Left.SelectedIndex = 1;
            AutoChange = false;
            return;
        }
        if(Left.SelectedIndex == Right.SelectedIndex)
        {
            AutoChange = true;
            if (Left.SelectedIndex != 6)
            {
                Left.SelectedIndex = 6;
            }
            else
                Left.SelectedIndex = 1;
            AutoChange = false;
        }

        AutoChange = true;
        LeftNumb.Text = "";
        AutoChange = true;
        RightNumb.Text = "";
        AutoChange = false;
        LeftNumb.IsEnabled = true;
        RightNumb.IsEnabled = true;
    }
    private bool AutoChange = false;

    private void LeftNumb_TextChanged(object sender, TextChangedEventArgs e)
    {
        if (AutoChange)
        {
            AutoChange = false;
            return;
        }
        decimal ch=0;
        if (LeftNumb.Text.Length == 0 || (LastCharCorrect(LeftNumb.Text) && decimal.TryParse(LeftNumb.Text, out ch)))
        {

            try
            {
                if (Left.SelectedIndex == 6)
                {
                    AutoChange = true;
                    RightNumb.Text = ((ch / _currencies[Right.SelectedIndex].Currency) * _currencies[Right.SelectedIndex].Scale).ToString();
                    AutoChange = false;
                }
                else
                {
                    AutoChange = true;
                    RightNumb.Text = ((ch * _currencies[Left.SelectedIndex].Currency) / _currencies[Left.SelectedIndex].Scale).ToString();
                    AutoChange = false;
                }
            }
            catch(Exception ex)
            {
                LeftNumbExc(e.OldTextValue);
            }

        }
        else
        {
            LeftNumbExc(e.OldTextValue);
        }
    }
    private void LeftNumbExc(string s)
    {
        AutoChange = true;
        LeftNumb.Text = s;
        AutoChange = false;
    }
    bool LastCharCorrect(string s)
    {
        bool point = s.Contains(".");
        char c = s[s.Length - 1];
        if ((c=='.'&&point)||!(c>='0'&&c<='9'))
        {
            return false;
        }
        return true;
    }

    private void RightNumb_TextChanged(object sender, TextChangedEventArgs e)
    {
        if (AutoChange)
        {
            AutoChange = false;
            return;
        }
        decimal ch=0;

        if (RightNumb.Text.Length==0 || (LastCharCorrect(RightNumb.Text)&&decimal.TryParse(RightNumb.Text, out ch)))
        {
            try
            {
                if (Right.SelectedIndex == 6)
                {
                    AutoChange = true;
                    LeftNumb.Text = ((ch / _currencies[Left.SelectedIndex].Currency) * _currencies[Left.SelectedIndex].Scale).ToString();
                    AutoChange = false;
                }
                else
                {
                    AutoChange = true;
                    LeftNumb.Text = ((ch * _currencies[Right.SelectedIndex].Currency) / _currencies[Right.SelectedIndex].Scale).ToString();
                    AutoChange = false;
                }
            }
            catch(Exception ex)
            {
                RightNumbExc(e.OldTextValue);
            }
        }
        else
        {
            RightNumbExc(e.OldTextValue);
        }
    }
    private void RightNumbExc(string s)
    {
        AutoChange = true;
        RightNumb.Text = s;
        AutoChange = false;
    }
}