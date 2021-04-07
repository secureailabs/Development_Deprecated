using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SailAuditInterface
{
    public class AuditEventNode
    {
        public string m_EventType;
        public string m_EventParentIdentifier;
        public string m_EventIdentifier;
        public uint m_SequenceNumber;
        public string m_EventEpochUtcTimestamp;
        public string m_EventName;
        public string m_EventData;
        public string m_EventText;
    }
}
