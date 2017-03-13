# automatically generated by the FlatBuffers compiler, do not modify

# namespace: 

import flatbuffers

class MonitorMessage(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsMonitorMessage(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = MonitorMessage()
        x.Init(buf, n + offset)
        return x

    # MonitorMessage
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # MonitorMessage
    def SourceName(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return ""

    # MonitorMessage
    def DataType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # MonitorMessage
    def Data(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            from flatbuffers.table import Table
            obj = Table(bytearray(), 0)
            self._tab.Union(obj, o)
            return obj
        return None

def MonitorMessageStart(builder): builder.StartObject(3)
def MonitorMessageAddSourceName(builder, sourceName): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(sourceName), 0)
def MonitorMessageAddDataType(builder, dataType): builder.PrependUint8Slot(1, dataType, 0)
def MonitorMessageAddData(builder, data): builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(data), 0)
def MonitorMessageEnd(builder): return builder.EndObject()