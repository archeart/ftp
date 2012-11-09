
int main()
{
	map_init();
	auth_init();

	printf("%d\n",get_uid_by_name("zwz"));
	printf("%d\n",get_uid_by_name("zwzmzd"));
	printf("%d\n",get_uid_by_name("nishi"));

	printf("%d\n",auth_user("zwz","aaa"));
	printf("%d\n",auth_user("iii","aaa"));
	printf("%d\n",auth_user("zwz","mzd"));

	auth_permission("/pro",1);
	auth_permission("/",1);
	auth_permission("/a/a/a/a/a",1);
	printf("%x\n",auth_permission("/pp",1));
	return 0;
}
