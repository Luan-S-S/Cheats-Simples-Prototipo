using _SimpleCheat;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;

namespace _SimpleCheat
{
    internal class Game : IDisposable
    {
        public Game(string name, string procName)
        {
            this.process = Process.GetProcessesByName(procName).FirstOrDefault();
            if (process == null)
            {
                throw new InvalidOperationException(@$"
             ╔════════════════════════════════════════╗
             ║         PROCESSO NÃO ENCONTRADO        ║
             ╚════════════════════════════════════════╝");
            }

            this.Name = name;
            this.ProcessName = procName;
            this.PID = (uint)process.Id;
            this.BaseAddress = process.MainModule.BaseAddress;
            this.hGame = Kernel32.OpenProcess(PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE, false, this.PID);
            if (this.hGame == IntPtr.Zero)
                throw new InvalidOperationException("Falha ao obter Handle para o processo: " + this.Name);
        }

        private const uint PROCESS_VM_OPERATION = 0x0008;
        private const uint PROCESS_VM_READ = 0x0010;
        private const uint PROCESS_VM_WRITE = 0x0020;
        private const uint PAGE_EXECUTE_READWRITE = 0x0040;
        private const uint PROCESS_QUERY_INFORMATION = 0x0400;

        public string Name { get; set; }
        public readonly string ProcessName;
        public readonly uint PID;
        public readonly IntPtr hGame;
        public readonly IntPtr BaseAddress;
        private Process process;

        public enum TypeModify
        {
            SingleModify,
            ToggleModify,
            InfinitModify
        }
        class Attribute
        {
            public string Name;
            public bool Modify;
            public object OriginalValue, CurrentValue, ModifyValue;
            public uint OriginalProtect, SizeValue;
            public int ConsolePosition;
            public IntPtr Address;
            public VirtualKey Key;
            public TypeModify TModify;
            public BitconverterMapping.ConvertBuffer ConvertBuffer;
            public BitconverterMapping.GetBuffer GetBuffer;
            public void Show(bool restorePosition)
            {
                string showString = $"{this.Address.ToString("X").PadRight(10)}  {this.Name.PadRight(20)} ->  {this.Key.ToString().PadRight(15)}: {this.CurrentValue.ToString()}";
                if (restorePosition)
                {
                    SimpleCheat.WriteRestoringPosition(this.ConsolePosition, showString);
                    return;
                }

                Console.WriteLine(showString);
            }
        }

        private List<Attribute> attributes = new List<Attribute>();

        private uint GetAttributeSizeValue(object attrValue)
        {
            if (attrValue is string str) return (uint)str.Length;
            if (attrValue is byte[] arr) return (uint)arr.Length;
            return (uint)Marshal.SizeOf(attrValue);
        }
        private bool ModifyMemoryProtectionAttribute(Attribute attr, uint flNewProtect)
        {
            return MemoryFunctions.ModifyMemoryProtect(this.hGame, attr.Address, attr.SizeValue, flNewProtect, out attr.OriginalProtect);
        }
        private IntPtr GetAttributeAddress(nint[] offsets)
        {
            nint address = (nint)this.BaseAddress;
            int size = offsets.Length;
            byte[] buffer;

            for (int i = 0; i < size; i++)
            {
                address += offsets[i];
                if (i < size - 1)
                {
                    buffer = MemoryFunctions.ReadMemoryValue(this.hGame, (IntPtr)address, sizeof(ulong));
                    if (buffer == null)
                        throw new InvalidOperationException("Falha ao ler endereço: " + address.ToString("X"));
                    address = IntPtr.Size == 8 ? (nint)BitConverter.ToUInt64(buffer) : (nint)BitConverter.ToUInt32(buffer);
                }
            }
            return (IntPtr)address;

        }
        private object GetAttributeValue(Attribute attr)
        {
            byte[] value = MemoryFunctions.ReadMemoryValue(this.hGame, attr.Address, attr.SizeValue);
            if (value == null)
                throw new InvalidOperationException($"Erro ao Ler atributo: {attr.Name} -> {attr.Address.ToString("X")}");
            return attr.ConvertBuffer(value, 0);
        }
        private bool SetAttributeValue(Attribute attr)
        {
            if (!MemoryFunctions.WriteMemoryValue(this.hGame, attr.Address, attr.GetBuffer(attr.ModifyValue)))
                throw new InvalidOperationException($"Erro ao modificar atributo: {attr.Name} -> {attr.Address.ToString("X")}");
            return true;
        }
        private bool SetAttributeToggleValue(Attribute attr)
        {
            object value = attr.Modify ? attr.ModifyValue : attr.OriginalValue;
            if (!MemoryFunctions.WriteMemoryValue(this.hGame, attr.Address, attr.GetBuffer(value)))
                throw new InvalidOperationException($"Erro ao modificar atributo: {attr.Name} -> {attr.Address.ToString("X")}");
            return true;
        }
        private bool SetAttributeValueByKey(Attribute attr)
        {
            if (SimpleCheat.KeyPressed(attr.Key))
            {
                attr.Modify = !attr.Modify;
                if (attr.TModify == TypeModify.SingleModify) return SetAttributeValue(attr);

                if (attr.TModify == TypeModify.ToggleModify) return SetAttributeToggleValue(attr);
            }
            if (attr.TModify == TypeModify.InfinitModify && attr.Modify && (!attr.CurrentValue.Equals(attr.ModifyValue))) return SetAttributeValue(attr);

            return true;
        }
        private bool RestoreAttribute(Attribute attr)
        {
            bool restoredValue = MemoryFunctions.WriteMemoryValue(this.hGame, attr.Address, attr.GetBuffer(attr.OriginalValue));
            return ModifyMemoryProtectionAttribute(attr, attr.OriginalProtect) && restoredValue;
        }
        public void ShowGame()
        {
            Console.WriteLine("--------------------------------------------------------------------\n" +
                              $"                          {this.Name}                            \n" +
                              "--------------------------------------------------------------------");
            Console.WriteLine($"Process Name: {this.ProcessName}.exe\n" +
                              $"PID: {this.PID}\n" +
                              $"Handle: 0x{this.hGame.ToString("X")}\n" +
                              $"Base Address: 0x{this.BaseAddress.ToString("X")}\n" +
                              "--------------------------------------------------------------------\n" +
                              "-------------------------- ATRIBUTOS -------------------------------\n" +
                              $"{"End.".PadRight(10)}  {"Nome".PadRight(20)} {"Tecla".PadRight(18)} Valor\n");

            //Mostrando atributos
            foreach (Attribute attr in attributes)
            {
                attr.ConsolePosition = Console.CursorTop;
                attr.Show(false);
            }
        }
        public void ShowAttributes()
        {
            object currentValue;
            foreach (Attribute attr in attributes)
            {
                if (!SetAttributeValueByKey(attr))
                    throw new InvalidOperationException("Falha ao modificar Atributo: " + attr.Name);

                currentValue = GetAttributeValue(attr);
                if (!currentValue.Equals(attr.CurrentValue))
                {
                    attr.CurrentValue = currentValue;
                    attr.Show(true);
                }
            }
        }
        public void AddAttribute(string name, nint[] offsets, VirtualKey key, TypeModify typeModify, object originalValue, object modifyValue)
        {
            if (originalValue == null || modifyValue == null)
                throw new InvalidOperationException($"O valor do atributo {name} não pode ser null");
            if (originalValue.GetType() != modifyValue.GetType())
                throw new InvalidOperationException("O tipo do valor original é diferente do tipo do modificado no atributo: " + name);
            Attribute attr = new Attribute()
            {
                Name = name,
                Address = GetAttributeAddress(offsets),
                Key = key,
                TModify = typeModify,
                OriginalValue = originalValue,
                ModifyValue = modifyValue,
                SizeValue = GetAttributeSizeValue(originalValue),
                Modify = false,
                ConvertBuffer = BitconverterMapping.GetFunctionFromBufferToObjectOfType(originalValue.GetType()),
                GetBuffer = BitconverterMapping.GetFunctionFromObjectToBufferOfType(originalValue.GetType())
            };
            ModifyMemoryProtectionAttribute(attr, PAGE_EXECUTE_READWRITE);
            attr.CurrentValue = GetAttributeValue(attr);
            this.attributes.Add(attr);
        }
        public bool HasExited() => process.HasExited;
        public void Dispose()
        {

            string attName = null;
            if (!this.HasExited())
            {
                foreach (Attribute attr in attributes)
                {
                    if (!RestoreAttribute(attr)) attName = attr.Name;
                }
            }
            Kernel32.CloseHandle(this.hGame);
            if (attName != null) throw new InvalidOperationException("Falha ao restaurar Atributo: " + attName);
        }
    }
}

static class MemoryFunctions
{
    public static byte[] ReadMemoryValue(IntPtr hProcess, IntPtr lpBaseAddress, uint size)
    {
        byte[] buffer = new byte[size];

        if (!Kernel32.ReadProcessMemory(hProcess, lpBaseAddress, buffer, size, out uint lpNumberOfBytesRead))
            return null;

        return buffer;
    }
    public static bool WriteMemoryValue(IntPtr hProcess, IntPtr lpBaseAddress, byte[] buffer)
    {
        uint size = Convert.ToUInt32(buffer.Length);
        return Kernel32.WriteProcessMemory(hProcess, lpBaseAddress, buffer, size, out uint lpNumberOfBytesWritten);
    }
    public static bool ModifyMemoryProtect(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flNewProtect, out uint lpflOldProtect)
    {
        return Kernel32.VirtualProtectEx(hProcess, lpAddress, dwSize, flNewProtect, out lpflOldProtect);
    }
}

static class BitconverterMapping
{
    public delegate object ConvertBuffer(byte[] value, int startIndex);
    public delegate byte[] GetBuffer(object value);

    private static Dictionary<Type, (ConvertBuffer, GetBuffer)> BufferConversions = new Dictionary<Type, (ConvertBuffer, GetBuffer)>()
    {
        [typeof(bool)] = ((b, i) => BitConverter.ToBoolean(b, i), (v) => BitConverter.GetBytes((bool)v)),
        [typeof(char)] = ((b, i) => BitConverter.ToChar(b, i), (v) => BitConverter.GetBytes((char)v)),
        [typeof(double)] = ((b, i) => BitConverter.ToDouble(b, i), (v) => BitConverter.GetBytes((double)v)),
        [typeof(Half)] = ((b, i) => BitConverter.ToHalf(b, i), (v) => BitConverter.GetBytes((Half)v)),
        [typeof(short)] = ((b, i) => BitConverter.ToInt16(b, i), (v) => BitConverter.GetBytes((short)v)),
        [typeof(int)] = ((b, i) => BitConverter.ToInt32(b, i), (v) => BitConverter.GetBytes((int)v)),
        [typeof(long)] = ((b, i) => BitConverter.ToInt64(b, i), (v) => BitConverter.GetBytes((long)v)),
        [typeof(float)] = ((b, i) => BitConverter.ToSingle(b, i), (v) => BitConverter.GetBytes((float)v)),
        [typeof(string)] = ((b, i) => Encoding.UTF8.GetString(b), (v) => Encoding.UTF8.GetBytes((string)v + "\0")),
        [typeof(ushort)] = ((b, i) => BitConverter.ToUInt16(b, i), (v) => BitConverter.GetBytes((ushort)v)),
        [typeof(uint)] = ((b, i) => BitConverter.ToUInt32(b, i), (v) => BitConverter.GetBytes((uint)v)),
        [typeof(ulong)] = ((b, i) => BitConverter.ToUInt64(b, i), (v) => BitConverter.GetBytes((ulong)v)),
        [typeof(byte[])] = ((b, i) => string.Join('|', b.Select(b => b.ToString("X2"))), b => (byte[])b)
    };
    private static (ConvertBuffer, GetBuffer) GetFunctionBitConvertFromType(Type type)
    {
        if (!BufferConversions.TryGetValue(type, out (ConvertBuffer, GetBuffer) convertBuffer))
            throw new InvalidOperationException("Tipo Invalido!");
        return convertBuffer;
    }
    public static ConvertBuffer GetFunctionFromBufferToObjectOfType(Type type)
    {
        return GetFunctionBitConvertFromType(type).Item1;
    }
    public static GetBuffer GetFunctionFromObjectToBufferOfType(Type type)
    {
        return GetFunctionBitConvertFromType(type).Item2;
    }
}