 internal abstract class AbstractBuilder
    {
        protected string name;
        protected Type type;
        protected IHousing house;

        public AbstractBuilder SetName(string name)
        {
            this.name = name;
            return this;
        }
        public AbstractBuilder SetType(Type t)
        {
            this.type = t;
            return this;
        }
        public AbstractBuilder SetHouse(IHousing house)
        {
            this.house = house;
            return this;
        }
        public abstract Housing Build();
    }
