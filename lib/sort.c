/*LInux内核中复杂度为O(nlogn)的快速，小型，非递归式排序*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sort.h>
#include <linux/slab.h>

/*将两通用指针类型指定的值互换（强制转换为u32类型的指针，然后互换值）*/
static void u32_swap(void *a, void *b, int size)
{
	u32 t = *(u32 *)a;
	*(u32 *)a = *(u32 *)b;
	*(u32 *)b = t;
}

/*将两个通用类型的指针的值（按指定字节长度）互换*/
static void generic_swap(void *a, void *b, int size)
{
	char t;

	do
	{
		t = *(char *)a;
		*(char *)a++ = *(char *)b;
		*(char *)b++ = t;
	} while (--size > 0);
}

/**
 * sort - sort an array of elements
 * @base: pointer to data to sort
 * @num: number of elements
 * @size: size of each element
 * @cmp: pointer to comparison function
 * @swap: pointer to swap function or NULL
 *
 * This function does a heapsort on the given array. You may provide a
 * swap function optimized to your element type.
 *
 * Sorting time is O(n log n) both on average and worst-case. While
 * qsort is about 20% faster on average, it suffers from exploitable
 * O(n*n) worst-case behavior and extra memory requirements that make
 * it less suitable for kernel use.
 */
void sort(void *base, size_t num, size_t size, int (*cmp)(const void *, const void *),
	  void (*swap)(void *, void *, int size))
{
	/*获取数组中间元素索引对应的偏移*/
	int i = (num/2 - 1) * size;
	/*获取数组最后一个元素对应的偏移*/
	int n = num * size;
	int c, r;
	/*测试交换函数指针是否存在，不存在时根据数组元素所占空间选择交换函数，数组元素
	占四个字节空间时使用u32_swap，否则使用generic_swap做为交换函数*/
	if (!swap)
		swap = (size == 4 ? u32_swap : generic_swap);

	/* heapify */
	/*从数组中间位置开始，每次移动到前一个元素起始位置*/
	for ( ; i >= 0; i -= size)
	{
		/*从数组中间元素开始，*/
		for (r = i; r * 2 + size < n; r  = c)
		{
			/**/
			c = r * 2 + size;
			/**/
			if (c < n - size && cmp(base + c, base + c + size) < 0)
				c += size;
			/**/
			if (cmp(base + r, base + c) >= 0)
				break;
			/**/
			swap(base + r, base + c, size);
		}
	}

	/*排序*/
	for (i = n - size; i > 0; i -= size)
	{
		swap(base, base + i, size);
		for (r = 0; r * 2 + size < i; r = c)
		{
			c = r * 2 + size;
			if (c < i - size && cmp(base + c, base + c + size) < 0)
				c += size;
			if (cmp(base + r, base + c) >= 0)
				break;
			swap(base + r, base + c, size);
		}
	}
}

EXPORT_SYMBOL(sort);

#if 0
/* a simple boot-time regression test */

int cmpint(const void *a, const void *b)
{
	return *(int *)a - *(int *)b;
}

static int sort_test(void)
{
	int *a, i, r = 1;

	a = kmalloc(1000 * sizeof(int), GFP_KERNEL);
	BUG_ON(!a);

	printk("testing sort()\n");

	for (i = 0; i < 1000; i++) {
		r = (r * 725861) % 6599;
		a[i] = r;
	}

	sort(a, 1000, sizeof(int), cmpint, NULL);

	for (i = 0; i < 999; i++)
		if (a[i] > a[i+1]) {
			printk("sort() failed!\n");
			break;
		}

	kfree(a);

	return 0;
}

module_init(sort_test);
#endif
