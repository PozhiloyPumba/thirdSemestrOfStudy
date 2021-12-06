#include <assert.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

//=====================================================================================================

const int ngroups_max = 100;

//=====================================================================================================

void print_id (struct passwd *user, struct group *group, int *groups, int ngroups);

//=====================================================================================================

int main (int argc, char *argv[])
{
    if (argc > 2) {
        printf ("Username mustn't have more than one word!\n");
        return 0;
    }
    if (argc == 1) {
        struct passwd *user = getpwuid (getuid ());
        struct group *group = getgrgid (getgid ());

        gid_t *groups = (gid_t *)calloc (ngroups_max, sizeof (gid_t));
        assert (groups);

        int ngroups = getgroups (ngroups_max, groups);

        print_id (user, group, groups, ngroups);

        free (groups);
    }
    else {
        struct passwd *user = getpwnam (argv[1]);
        if (!user) {
            printf ("User with this name was not found\n");
            return 0;
        }
        struct group *group = getgrgid (user->pw_gid);

        gid_t *groups = (gid_t *)calloc (ngroups_max, sizeof (gid_t));
        assert (groups);

        int ngroups = ngroups_max;
        getgrouplist (group->gr_name, group->gr_gid, groups, &ngroups);

        print_id (user, group, groups, ngroups);
        free (groups);
    }

    return 0;
}

void print_id (struct passwd *user, struct group *group, int *groups, int ngroups)
{
    printf ("uid=%d(%s) gid=%d(%s) groups=%d(%s)",
            user->pw_uid,
            user->pw_name ? user->pw_name : "",
            group->gr_gid,
            group->gr_name ? group->gr_name : "",
            group->gr_gid,
            group->gr_name ? group->gr_name : "");

    int gid = group->gr_gid;

    for (int i = 0; i < ngroups; i++) {
        struct group *other_group = getgrgid (groups[i]);

        if (gid != other_group->gr_gid)
            printf (",%d(%s)", other_group->gr_gid, other_group->gr_name ? other_group->gr_name : "");
    }
    printf ("\n");
}