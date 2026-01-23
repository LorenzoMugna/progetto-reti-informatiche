#let document(
  title: content,
  subtitle: content,
  doc,
) = {
  set page(paper: "a4", columns: 2, margin: (x: 2cm, y: 2cm))

  set text(
    lang: "it",
    font: "New Computer Modern",
    size: 11pt,
  )

  let SPACE_LEADING = 0.55em
  set par(
    justify: true,
    first-line-indent: 1.8em,
    spacing: SPACE_LEADING,
    leading: SPACE_LEADING,
  )

	show heading.where(level: 2): set text(weight: "regular")

  place(float: true, scope: "parent", center + top)[
		#set text(size: 18pt)
		*#title*

		#set text(size: 14pt)
		#subtitle
		
		#set text(size: 12pt)
		Appello del 17 febbraio 2026
  ]

  doc
}
