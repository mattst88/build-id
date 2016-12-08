SECTIONS
{
	__note_gnu_build_id_start = .;
}
INSERT BEFORE .note.gnu.build-id;

SECTIONS
{
	__note_gnu_build_id_end = .;
}
INSERT AFTER .note.gnu.build-id;
