#ifndef _LINUX_PID_NS_H
#define _LINUX_PID_NS_H

#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/threads.h>
#include <linux/nsproxy.h>
#include <linux/kref.h>

struct pidmap {
       atomic_t nr_free;
       void *page;
};

#define PIDMAP_ENTRIES         ((PID_MAX_LIMIT + 8*PAGE_SIZE - 1)/PAGE_SIZE/8)

struct bsd_acct_struct;

struct pid_namespace {
	struct kref kref;
	struct pidmap pidmap[PIDMAP_ENTRIES];
	int last_pid;
	int dead;
	struct task_struct *child_reaper;
	struct kmem_cache *pid_cachep;
	unsigned int level;
	unsigned int proc_inum;
	struct pid_namespace *parent;
#ifdef CONFIG_PROC_FS
	struct vfsmount *proc_mnt;
#endif
#ifdef CONFIG_BSD_PROCESS_ACCT
	struct bsd_acct_struct *bacct;
#endif
};

extern struct pid_namespace init_pid_ns;

#ifdef CONFIG_PID_NS
static inline struct pid_namespace *get_pid_ns(struct pid_namespace *ns)
{
	if (ns != &init_pid_ns)
		kref_get(&ns->kref);
	return ns;
}

extern struct pid_namespace *copy_pid_ns(unsigned long flags,
	struct pid_namespace *default_ns, struct pid_namespace *active_ns);
extern void free_pid_ns(struct kref *kref);
extern void zap_pid_ns_processes(struct pid_namespace *pid_ns);

static inline void put_pid_ns(struct pid_namespace *ns)
{
	if (ns != &init_pid_ns)
		kref_put(&ns->kref, free_pid_ns);
}

static inline bool pid_ns_dead(struct pid *pid)
{
	int i;

	for (i = 0; i <= pid->level; i++) {
		if (pid->numbers[i].ns->dead)
			return true;
	}
	return false;
}

#else /* !CONFIG_PID_NS */
#include <linux/err.h>

static inline struct pid_namespace *get_pid_ns(struct pid_namespace *ns)
{
	return ns;
}

static inline struct pid_namespace *copy_pid_ns(unsigned long flags,
	struct pid_namespace *default_ns, struct pid_namespace *active_ns)
{
	if (flags & CLONE_NEWPID)
		return ERR_PTR(-EINVAL);
	return default_ns;
}

static inline void put_pid_ns(struct pid_namespace *ns)
{
}


static inline void zap_pid_ns_processes(struct pid_namespace *ns)
{
	BUG();
}

static inline bool pid_ns_dead(struct pid *pid)
{
	return false;
}

#endif /* CONFIG_PID_NS */

extern struct pid_namespace *task_active_pid_ns(struct task_struct *tsk);
void pidhash_init(void);
void pidmap_init(void);

#endif /* _LINUX_PID_NS_H */
