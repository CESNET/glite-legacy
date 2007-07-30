#define glite_lb_padded_struct(_padded_name,_padded_size,_padded_content) \
	struct _padded_name##_to_pad__dont_use { _padded_content }; \
	struct _padded_name { \
		_padded_content \
		char _padding[_padded_size*sizeof(void *) - sizeof(struct _padded_name##_to_pad__dont_use)]; \
	};
