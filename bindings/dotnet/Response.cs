#pragma warning disable CS0649 // Field is never assigned to, and will always have its default value
namespace Preagonal.Scripting.GS2Compiler
{
	internal struct Response
	{
		public bool   Success;
		public string ErrMsg;
		public IntPtr ByteCode;
		public uint   ByteCodeSize;
	}
}