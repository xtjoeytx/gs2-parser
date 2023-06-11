using System.Runtime.InteropServices;

namespace Gs2Compiler
{
	public static class Interface
	{
		[DllImport("gs2compiler", CallingConvention = CallingConvention.Cdecl)]
		private static extern IntPtr get_context();

		[DllImport("gs2compiler", CallingConvention = CallingConvention.Cdecl)]
		private static extern Response compile_code(IntPtr context, string? code, string? type, string? name);

		[DllImport("gs2compiler", CallingConvention = CallingConvention.Cdecl)]
		private static extern void delete_context(IntPtr context);

		public static CompilerResponse CompileCode(string? code, string? type = "weapon", string? name = "npc")
		{
			IntPtr context = get_context();
			Response response = compile_code(context, code, type, name);

			CompilerResponse compilerResponse = new()
			{
				Success = response.Success,
				ErrMsg = response.ErrMsg,
			};

			if (response.ByteCodeSize > 0)
			{
				compilerResponse.ByteCode = new byte[response.ByteCodeSize];
				Marshal.Copy(response.ByteCode, compilerResponse.ByteCode, 0, (int)response.ByteCodeSize);
			}

			delete_context(context);

			return compilerResponse;
		}
	}
}