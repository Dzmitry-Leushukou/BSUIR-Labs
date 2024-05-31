internal abstract class Housing
{
    protected string name;
    protected Type type;
    private IHousing house;

    public string Name
    {
        get { return name; }
        set { name = value; }
    }

    public Housing(string name, Type type, IHousing house)
    {
        Name = name;
        this.type = type;
        this.house = house;
    }

    public abstract void GetInfo();

    public void SetHouse(IHousing house) 
    {
        this.house = house;
    }

}

