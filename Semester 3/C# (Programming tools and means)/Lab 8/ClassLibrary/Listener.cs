public class Listener
{
        public int Id { get; set; }
        public string Name { get; set; }
        public int CourseCount { get; set; }

        public override string ToString()
        {
            return $"[{Id}] {Name} : {CourseCount}";
        }

}