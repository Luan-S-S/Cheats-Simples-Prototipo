namespace _SimpleCheat
{
    public enum VirtualKey
    {
        A = 0x41,
        B = 0x42,
        C = 0x43,
        D = 0x44,
        E = 0x45,
        F = 0x46,
        G = 0x47,
        H = 0x48,
        I = 0x49,
        J = 0x4A,
        K = 0x4B,
        L = 0x4C,
        M = 0x4D,
        N = 0x4E,
        O = 0x4F,
        P = 0x50,
        Q = 0x51,
        R = 0x52,
        S = 0x53,
        T = 0x54,
        U = 0x55,
        V = 0x56,
        W = 0x57,
        X = 0x58,
        Y = 0x59,
        Z = 0x5A,

        Enter = 0x0D,
        Shift = 0x10,
        Control = 0x11,
        Alt = 0x12,
        Space = 0x20,
        Escape = 0x1B,
        Tab = 0x09,
        Backspace = 0x08,
        CapsLock = 0x14,
        End = 0x23,
        Delete = 0x2E,
    }

    static class SimpleCheat
    {
        static void Main()
        {
            DrawMessage(@"
            ░█████╗░██╗░░██╗███████╗░█████╗░████████╗
            ██╔══██╗██║░░██║██╔════╝██╔══██╗╚══██╔══╝
            ██║░░╚═╝███████║█████╗░░███████║░░░██║░░░
            ██║░░██╗██╔══██║██╔══╝░░██╔══██║░░░██║░░░
            ╚█████╔╝██║░░██║███████╗██║░░██║░░░██║░░░
            ░╚════╝░╚═╝░░╚═╝╚══════╝╚═╝░░╚═╝░░░╚═╝░░░", ConsoleColor.Red);


            Game game = null;

            try
            {
                game = new Game("ASSAULT CUBE 1.3.0.2", "ac_client");

                game.AddAttribute("Vida", new nint[] { 0x18AC00, 0xEC }, VirtualKey.V, Game.TypeModify.InfinitModify, 100, 9999);
                game.AddAttribute("Munição Rifle", new nint[] { 0x18AC00, 0x140 }, VirtualKey.R, Game.TypeModify.SingleModify, 20, 9999);
                game.AddAttribute("Nome", new nint[] { 0x18AC00, 0x205 }, VirtualKey.N, Game.TypeModify.ToggleModify, "urnamed", "Luan    ");
                game.AddAttribute("No Recoil", new nint[] { 0xC2EC3 }, VirtualKey.T, Game.TypeModify.ToggleModify, new byte[] { 0xF3, 0x0F, 0x11, 0x56, 0x38 }, new byte[] { 0x90, 0x90, 0x90, 0x90, 0x90 });

                game.ShowGame();
                while (!KeyPressed(VirtualKey.End) && !game.HasExited())
                {
                    game.ShowAttributes();
                    Thread.Sleep(100);
                }

            }
            catch (InvalidOperationException ex)
            {

                Console.Clear();
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine(ex.Message);
                Thread.Sleep(3000);
                Console.ResetColor();
            }
            finally
            {
                game?.Dispose();
                Console.Clear();
                DrawMessage(@"
            ╔════════════════════════════════════════╗
            ║            CHEAT FINALIZADO            ║
            ╚════════════════════════════════════════╝", ConsoleColor.Green);

            }
        }
        public static bool KeyPressed(VirtualKey key)
        {
            bool pressed = false;

            while ((User32.GetAsyncKeyState((int)key) & 0x8000) != 0)
                pressed = true;

            return pressed;
        }
        public static void DrawMessage(string message, ConsoleColor color)
        {
            Console.ForegroundColor = color;
            Console.WriteLine(message);
            Console.ResetColor();
        }
        public static void WriteRestoringPosition(int linePosition, string msg)
        {
            int originalPosition = Console.CursorTop;
            Console.CursorTop = linePosition;
            ClearCurrentConsoleLine();
            Console.WriteLine(msg);
            Console.CursorTop = originalPosition;
        }
        public static void ClearCurrentConsoleLine()
        {
            Console.Write('\r' + new String(' ', Console.WindowWidth) + '\r');
        }
    }

}