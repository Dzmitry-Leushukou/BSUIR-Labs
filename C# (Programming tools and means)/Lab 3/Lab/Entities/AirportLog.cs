using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab.Entities
{
    internal class AirportLog
    {
        public AirportLog(DateTime date, string comment = "")
        {
            Date = date;
            Comment = comment;
        }
        public DateTime Date { get; private set; }
        public string Comment { get; private set; }
    }
}
