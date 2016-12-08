SECTIONS
{
	build-id : {
		PROVIDE(__note_gnu_build_id_start = .);
		*(.note.gnu.build-id);
		PROVIDE(__note_gnu_build_id_end = .);
	}
}
INSERT AFTER .note.gnu.build-id;
