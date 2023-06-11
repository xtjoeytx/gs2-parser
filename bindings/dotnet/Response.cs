namespace Gs2Compiler
{
	internal struct Response
	{
		public bool   Success;
		public string ErrMsg;
		public IntPtr ByteCode;
		public uint   ByteCodeSize;
	}
}