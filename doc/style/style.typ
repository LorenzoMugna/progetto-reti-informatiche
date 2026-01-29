#let numbering_offset(offset: int, ..numbers) = {
  let nums = numbers.pos()

  if nums.len() < offset + 1 {
    return
  }

  nums = nums.slice(offset)
  numbering("1.1", ..nums)
}

#let SPACE_LEADING = 0.55em
#let HEADING_OFFSET = 0
#let document(
  title: content,
  subtitle: content,
  doc,
) = {
  set page(
    paper: "a4",
    columns: 2,
    margin: (x: 2cm, y: 2cm),
    header: [Lorenzo Mugnaioli -- Reti Informatiche -- 17 febbraio 2026 #line(end: (100%, 0%))],
  )

  set text(
    lang: "it",
    font: "New Computer Modern",
    size: 11pt,
  )

  set heading(offset: HEADING_OFFSET)
  set heading(numbering: numbering_offset.with(offset: HEADING_OFFSET))

  place(float: true, scope: "parent", center + top)[
    #set text(size: 18pt)
    *#title*

  ]
  [
    #set text(size: 14pt)
    #subtitle \
    #set text(size: 12pt)
    Appello del 17 febbraio 2026

    #line(end: (100%, 0%))

  ]

  set par(
    justify: true,
    first-line-indent: 1.8em,
    spacing: SPACE_LEADING,
    leading: SPACE_LEADING,
  )
  doc
}
